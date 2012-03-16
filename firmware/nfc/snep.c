/*
 * Copyright 2012 Google Inc. All Rights Reserved.
 * Author: ghohpe@google.com (Gregor Hohpe)
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
 * See http://www.nfc-forum.org/specs/spec_list/
 */

#include "snep.h"

#define SNEP_VERSION 0x10

// Commands
#define SNEP_GET 0x01
#define SNEP_PUT 0x02

/*
 * Writes a SNEP header for a payload of up to 255 bytes.
 * Returns the number of bytes written.
 *
 * 0x00: Version (major/minor)
 * 0x01: Command
 * 0x02-0x05: Payload length MSB first
 */
uint8_t snep_put(uint8_t *buf, uint8_t data_len)
{
  buf[0] = SNEP_VERSION;
  buf[1] = SNEP_PUT;
  buf[2] = 0x00;
  buf[3] = 0x00;
  buf[4] = 0x00;
  buf[5] = data_len;
  return 6;
}

/*
 * Returns the status byte of a SNEP response message.
 *
 * 0x00: Version (major/minor)
 * 0x01: Response Status Code
 */
uint8_t snep_response_status(uint8_t *buf)
{
  return buf[1];
}
