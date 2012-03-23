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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "../peripheral/lcd.h"
#include "rcs956_protocol.h"

#include "rcs956_common.h"

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
