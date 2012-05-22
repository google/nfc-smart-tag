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
 * Common commands for RC-S956 NFC Chip.
 *
 * See http://www.sony.co.jp/Products/felica/business/tech-support
 *
 */

#include <string.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "../peripheral/lcd.h"
#include "../peripheral/usart.h"
#include "rcs956_protocol.h"

#include "rcs956_common.h"

#define CMD 0xd4
#define COMM_THRU_EX 0xa0

/*
 * Sends data via NFC and receives a response.
 *
 * Parameters:
 *   payload: Data to be sent via NFC
 *   payload_len: Length of payload
 *   resp: Response buffer for data received via NFC
 *   resp_len: Size of response buffer
 *   timeout: timeout limit in ms (max 30000)
 *
 * Returns:
 *   Length of received payload data
 */
int rcs956_comm_thru_ex(uint8_t *payload, size_t payload_len,
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
 * Sets the NFC module into mode 0.
 * Returns true on success.
 */
bool rcs956_reset(void)
{
  static const prog_char __cmd[] = {0xd4, 0x18, 0x01};
  uint8_t resp[MAX_RECV_SIZE];

  if (!rcs956_send_command_p(__cmd, sizeof(__cmd))) {
    lcd_printf(0, "reset fail");
    return false;
  }

  if (!rcs956_read_response(resp, sizeof(resp))) {
    lcd_printf(1, "rst rs fail %d %02X", resp[3], resp[5]);
    return false;
  }

  // Reset command requires host to ACK and wait > 10ms
  rcs956_cancel_cmd();
  _delay_ms(10);

  return true;
}

/*
 * Wakes up NFC module from soft power down within 1 + 110000/serial_bps ms.
 */
void rcs956_serial_wake_up(void)
{
  static const prog_char __cmd_wake_up[] = {0x55};

  usart_send_buf_p(__cmd_wake_up, (int)sizeof(__cmd_wake_up));
  _delay_ms(2);
}
