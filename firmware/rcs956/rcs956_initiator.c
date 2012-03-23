/*
 * Copyright 2012 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Commands to control the RC-956 NFC chip as initiator.
 */

#include <string.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "rcs956_protocol.h"
#include "../peripheral/lcd.h"

#include "rcs956_initiator.h"

#define L8(x) ((x) & 0xff)
#define H8(x) (((x) >> 8) & 0xff)

#define CMD 0xd4
#define RF_CONFIG 0x32
#define COMM_THRU 0x42
#define COMM_THRU_EX 0xa0
#define LIST_TGT 0x4a

/**
 * Turn off RF field.
 *
 * Commands that need RF turn the field back on automatically.
 */
void rcs956_rf_off(void)
{
  uint8_t resp[MAX_RECV_SIZE];

  static const prog_char __cmd_rf_off[] = {CMD, RF_CONFIG, 0x01, 0x00};

  (void)rcs956_send_command_p(__cmd_rf_off, sizeof(__cmd_rf_off));
  (void)rcs956_read_response(resp, sizeof(resp));
}

/**
 * Checks whether a card (phone) is present, If so, returns true
 * and fills the idm and pmm buffers.
 *
 * returns true if card (phone) successfully detected
 *         false if no card detected (timeout) or error occurred
 */
bool initiator_poll(uint8_t idm[], uint8_t pmm[], uint16_t syscode)
{
  uint8_t cmd[MAX_SEND_SIZE];
  uint8_t resp[MAX_RECV_SIZE];

  // Felica InListPassiveTarget Request
  // 0x00: 0xd4 Command Code
  // 0x01: 0x4a Subcommand Code
  // 0x02: 0x01 MaxTg Max number of targets
  // 0x03: 0x01 BRTY Baud Rate and Communication Mode = ISO 18092
  //
  // Sony Felica Card User's Manual
  // (NFCIP-1 Polling Request Frame Format ECMA-340 Sec 11.2.2.5)
  // 0x04: 0x00 Command Code
  // 0x05:      System Code high byte
  // 0x06:      System Code low byte
  // 0x07: 0x00 Request Code: "No Request"
  // 0x08: 0x00 TSN - Time Slot. 0 = only single time slot
  //
  //
  // RC-956 Normal Frame
  // 0x00-0x04: RC-956 Envelope
  //
  // Felica InListPassiveTarget Response
  // 0x05: 0xd5 Command Code
  // 0x06: 0x4b Subcommand Code
  // 0x07:      NbTg Number of targets
  // 0x08: 0x01 Logical number of target
  // 0x09: 0x12 Length of polling response (0x12 for "No Request")
  //
  // Sony Felica Card User's Manual
  // 0x0a: 0x01 Response Code to Polling command
  // 0x0b-0x12: IDm (Manufacture ID)
  // 0x13-0x1a: PMm (Manufacture Parameter)

  static const prog_char cmd_poll_prefix[] = {CMD, LIST_TGT, 0x01, 0x01, 0x00};
  static const prog_char cmd_poll_suffix[] = {0x00, 0x00};

  memcpy_P(cmd, cmd_poll_prefix, sizeof(cmd_poll_prefix));
  cmd[sizeof(cmd_poll_prefix)] = H8(syscode);
  cmd[sizeof(cmd_poll_prefix)+1] = L8(syscode);
  memcpy_P(&cmd[sizeof(cmd_poll_prefix) + 2], cmd_poll_suffix,
           sizeof(cmd_poll_suffix));

  if (!rcs956_send_command(cmd,
                           sizeof(cmd_poll_prefix)
                           + sizeof(cmd_poll_suffix) + 2)) {
    return false;
  }

  if (!rcs956_read_response(resp, sizeof(resp))) {
    return false;
  }

  // If no card found (NbTg = 0), just return
  if (resp[7] != 0x01) {
    return false;
  }

  memcpy(idm, &resp[0x0b], 8);
  if (pmm != NULL)
    memcpy(pmm, &resp[0x13], 8);

  return true;
}

/*
 * Sends data to the NFC module and receives a response.
 *
 * Parameters:
 *   payload: Felica push command
 *   payload_len: Length of Felica Push command
 *   resp: Response buffer
 *   resp_len: Size of response buffer
 *   timeout: timeout limit in ms (max 30000)
 *
 * Returns:
 *   Length of received payload data
 */
int initiator_command(uint8_t *payload, size_t payload_len,
                      uint8_t *resp, size_t resp_len,
                      uint16_t timeout)
{
  static const prog_char __cmd[] = {CMD, COMM_THRU_EX};

  uint8_t cmd[MAX_SEND_SIZE];
  uint8_t *idx = cmd;

  if (payload_len > sizeof(cmd) - sizeof(__cmd)) {
    protocol_errno = BUFFER_EXCEEDED;
    return 0;
  }

  memcpy_P(cmd, __cmd, sizeof(__cmd));
  idx += sizeof(__cmd);
  // Time-out in 0.5ms increments (multiply by 2 with left shift)
  *idx++ = L8(timeout << 1);
  *idx++ = H8(timeout << 1);
  memcpy(idx, payload, payload_len);
  idx += payload_len;

  if (!rcs956_send_command(cmd, idx - cmd)) {
    lcd_printf(0, "ctex send fail");
    return 0;
  }

  if (!rcs956_read_response(resp, resp_len)) {
    lcd_printf(0, "ctex resp fail %d", resp[OFS_DATA_LEN]);
    return 0;
  }

  if (resp[OFS_DATA] != 0x00) { // First byte is status
    lcd_printf(0, "ctex st fail %02X", resp[OFS_DATA]);
    protocol_errno = UNEXPECTED_REPLY;
    return 0;
  }

  return resp[OFS_DATA_LEN];
}

/*
 * Sends a command, waits for ACK, and reads response. Times out after
 * USART_READ_TIMEOUT.
 */
bool __execute_command(
    uint8_t *cmd, int cmd_size, uint8_t *resp, int resp_size)
{
  if (!rcs956_send_command(cmd, cmd_size)) {
    return false;
  }

  return rcs956_read_response(resp, resp_size);
}

/*
 * Defines the retry count for RF communication for InListPassiveTarget,
 * i.e. when polling for a device.
 * 0 = no retry (just do it once), 0xff = infinite retry
 */
bool rcs956_set_retry(uint8_t retry)
{
  static const prog_char __cmd_rf_retry[] = {CMD, RF_CONFIG, 0x05};
  uint8_t cmd[6];
  uint8_t resp[MAX_RECV_SIZE];

  memcpy_P(cmd, __cmd_rf_retry, sizeof(__cmd_rf_retry));
  cmd[3] = retry; // ATR_REQ, we do not use
  cmd[4] = 0x00; // PSL_REQ, 0 = default
  cmd[5] = retry; // InListPassiveTarget

  return __execute_command(cmd, sizeof(cmd), resp, sizeof(resp));
}

/*
 * Defines the retry count for RF communication for InCommunicateThrough
 * command (e.g., for pushing URL).
 * 0 = no retry (just do it once), 0xff = infinite retry
 */
bool rcs956_set_retry_com(uint8_t retry)
{
  static const prog_char __cmd_rf_retry_com[] = {CMD, RF_CONFIG, 0x04};
  uint8_t cmd[4];
  uint8_t resp[MAX_RECV_SIZE];

  memcpy_P(cmd, __cmd_rf_retry_com, sizeof(__cmd_rf_retry_com));
  cmd[3] = retry;

  return __execute_command(cmd, sizeof(cmd), resp, sizeof(resp));
}

/*
 * Sets RF Communication Timeout Value used by InCommunicateThru.
 * This is needed to push URL to Felica Androids.
 */
bool rcs956_set_timeout(uint8_t timeout)
{
  static const prog_char __cmd_timeout[] = {CMD, RF_CONFIG, 0x02};
  uint8_t cmd[6];
  uint8_t resp[MAX_RECV_SIZE];

  memcpy_P(cmd, __cmd_timeout, sizeof(__cmd_timeout));
  cmd[3] = 0x0b; // PSL_RES timeout (default)
  cmd[4] = 0x0b; // ATR_RES timeout (default)
  cmd[5] = timeout; // Set RC Communication timeout value

  return __execute_command(cmd, sizeof(cmd), resp, sizeof(resp));
}
