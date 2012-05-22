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
 * Commands to control the RC-956 NFC chip as target or peer-to-peer.
 *
 * http://www.sony.co.jp/Products/felica/business/tech-support
 */

#include <string.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "rcs956_common.h"
#include "rcs956_protocol.h"
#include "../peripheral/lcd.h"
#include "../peripheral/power_down.h"
#include "../peripheral/usart.h"

#include "rcs956_target.h"


/*
 * Writes a configuration register value.
 */
bool rcs956_write_register(uint16_t adr, uint8_t val)
{
  static const prog_char __cmd[] = {0xd4, 0x08};
  uint8_t resp[MAX_RECV_SIZE];
  uint8_t cmd[MAX_SEND_SIZE];

  memcpy_P(cmd, __cmd, sizeof(__cmd));
  cmd[sizeof(__cmd)] = H8(adr);
  cmd[sizeof(__cmd)+1] = L8(adr);
  cmd[sizeof(__cmd)+2] = val;

  if (!rcs956_send_command(cmd, sizeof(__cmd)+3)) {
    lcd_printf(0, "wreg %x fail", adr);
    return false;
  }

  if (!rcs956_read_response(resp, sizeof(resp))) {
    lcd_printf(0, "wreg rs fail %d %02X", resp[3], resp[5]);
    return false;
  }

  if (resp[7] != 0x00) {
    lcd_printf(0, "wreg fail %02X", resp[7]);
    protocol_errno = UNEXPECTED_REPLY;
    return false;
  }

  return true;
}

/*
 * Set parameters:
 * b4: Automatic RATS (1 default)
 * b3: fUsbSuspendRFLD (1 default)
 * b2: Automatic ATR_RES (1 default)
 */
bool rcs956_set_param(uint8_t flags)
{
  static const prog_char __cmd[] = {0xd4, 0x12};
  uint8_t resp[MAX_RECV_SIZE];
  uint8_t cmd[MAX_SEND_SIZE];

  memcpy_P(cmd, __cmd, sizeof(__cmd));
  cmd[sizeof(__cmd)] = flags;

  if (!rcs956_send_command(cmd, sizeof(__cmd)+1)) {
    return false;
  }

  if (!rcs956_read_response(resp, sizeof(resp))) {
    lcd_printf(0, "sp rs fail %d %02X", resp[3], resp[5]);
    return false;
  }

  return true;
}

/*
 * Set the RC-S620 into target mode, ready to receive data from
 * an initiator.
 */
int rcs956_tg_init(const uint8_t idm[])
{
  static const prog_char __cmd_prefix[] = {
    0xd4, 0x8c,
    0x00, // Activated
    0x01, 0x01, /* sens_res 2bytes */
    0x00, 0x00, 0x00, /* nfcid 3bytes */
    0x40, /* SEL_RES */
  };

  static const prog_char __extra_parms[] = {
    0x01, 0x20, 0x22, 0x04, 0x27, 0x3f, 0x7f, 0xff,
    0x12, 0xfc
  };

  uint8_t cmd[MAX_SEND_SIZE];
  size_t cmd_len = 0;

  // Header and 106kbps Params
  memcpy_P(&cmd[cmd_len], __cmd_prefix, sizeof(__cmd_prefix));
  cmd_len += sizeof(__cmd_prefix);

  // 212/424 kbps params
  memcpy(&cmd[cmd_len], idm, 8);
  cmd_len += 8;
  memcpy_P(&cmd[cmd_len], __extra_parms, sizeof(__extra_parms));
  cmd_len += sizeof(__extra_parms);

  /* NFCID3 10 bytes */
  memset(&cmd[cmd_len], 0x00, 10);
  cmd_len += 10;

  if (!rcs956_send_command(cmd, cmd_len)) {
    lcd_printf(0, "tgi send fail %i", protocol_errno);
    return 0;
  }
  lcd_printf(0, "tgi sent");
  return 1;
}

/*
 * Waits for initiator to connect to RC-S620.
 *
 * Returns: 1 on success, -1 on timeout, 0 on fail
 */
int rcs956_tg_wait_initiator(uint8_t *resp, size_t resp_len)
{
  uint8_t count;
  // Loop because max sleep time on 8 bit timer is less than needed
  for (count = 0; count < SLEEP_COUNT(TG_INIT_WAIT_MS); count++) {
    // Wake up on data (USART interrupt) or time out (Timer interrupt)
    sleep_until_timer(SLEEP_MODE_IDLE, false);
    if (usart_has_data()) {
      if (!rcs956_read_response(resp, resp_len)) {
        lcd_printf(0, "tgi resp fail %d", resp[3]);
        return 0;
      }
      return 1;
    }
  }
  rcs956_serial_wake_up(); // NFC Module may be powered down
  rcs956_cancel_cmd();
  return -1;
}

/*
 * Sets the general bytes for ATR_RES.
 */
bool rcs956_tg_set_general_bytes(uint8_t *payload, size_t payload_len)
{
  static const prog_char __cmd[] = {0xd4, 0x92};
  uint8_t cmd[MAX_SEND_SIZE];
  uint8_t resp[MAX_RECV_SIZE];
  size_t cmd_len = payload_len + 2;

  if (payload_len > sizeof(cmd) - sizeof(__cmd)) {
    protocol_errno = BUFFER_EXCEEDED;
    return false;
  }

  memcpy_P(cmd, __cmd, sizeof(__cmd));
  memcpy(&cmd[2], payload, payload_len);

  if (!rcs956_send_command(cmd, cmd_len)) {
    lcd_printf(0, "tsgb send fail");
    return false;
  }

  if (!rcs956_read_response(resp, sizeof(resp))) {
    lcd_printf(0, "sgb rs fail %d %02X", resp[OFS_DATA_LEN], resp[5]);
    return false;
  }

  if (resp[OFS_DATA] != 0x00) {
    lcd_printf(0, "tsgb st fail %02X", resp[OFS_DATA]);
    protocol_errno = UNEXPECTED_REPLY;
    return false;
  } else {
    return true;
  }
}

/*
 * Receives data in ISO18092 peer-to-peer mode (DEP_REQ).
 * Returns data size.
 */
int rcs956_tg_get_dep_data(uint8_t *resp, size_t resp_len)
{
  static const prog_char __cmd[] = {0xd4, 0x86};
  if (!rcs956_send_command_p(__cmd, sizeof(__cmd))) {
    lcd_printf(0, "getdep tx fail");
    return 0;
  }

  if (!rcs956_read_response(resp, resp_len)) {
    lcd_printf(0, "getdep rx fail %d", resp[OFS_DATA_LEN]);
    lcd_printf(1, "err %i", protocol_errno);
    return 0;
  }
  // TODO: if bit 6 of status is set, re-execute (chaining)
  return resp[OFS_DATA_LEN];
}

/*
 * Sends data in ISO18092 peer-to-peer mode (DEP_RES).
 * Returns true & sets status on success with RC-620.
 * Status indicates protocol errors or success.
 */
bool rcs956_tg_set_dep_data(uint8_t *data, size_t data_len, uint8_t *status)
{
  static const prog_char __cmd[] = {0xd4, 0x8e};

  uint8_t cmd[MAX_SEND_SIZE];
  uint8_t resp[MAX_RECV_SIZE];

  if (data_len > sizeof(cmd) - sizeof(__cmd)) {
    protocol_errno = BUFFER_EXCEEDED;
    return false;
  }

  memcpy_P(cmd, __cmd, sizeof(__cmd));
  memcpy(&cmd[2], data, data_len);

  if (!rcs956_send_command(cmd, data_len + 2)) {
    lcd_printf(0, "setdep tx fail");
    return false;
  }

  if (!rcs956_read_response(resp, sizeof(resp))) {
    lcd_printf(0, "setdep rx fail %d", resp[OFS_DATA_LEN]);
    lcd_print_hex(1, resp, 8);
    return false;
  }
  *status = resp[OFS_DATA];
  return true;
}
