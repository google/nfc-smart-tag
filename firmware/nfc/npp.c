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
 *
 * Support for NPP (NDEF Push Protocol).
 *
 * See source.android.com/compatibility/ndef-push-protocol.pdf
 */

#include <string.h>

#include <avr/pgmspace.h>

static prog_char service_name[] = "com.android.npp";

/*
 * Fills a buffer with the LLCP service name for NPP, including \0.
 */
prog_char *get_npp_service_name(void)
{
  return service_name;
}

/*
 * Creates an NPP command from a single NDEF record.
 *
 * Arguments:
 *  buf - output buffer to receive the NPP command
 *  ndef - NDEF record data
 *  ndef_len - Length of ndef data in bytes
 */
uint8_t npp(uint8_t *buf, uint8_t *ndef, uint8_t ndef_len) {
  uint8_t *p = buf;

  *p++ = 0x01; // Version
  *p++ = 0x00; // Number of NDEF records in big endian
  *p++ = 0x00;
  *p++ = 0x00;
  *p++ = 0x01;
  *p++ = 0x01; // NDEF action (0x01 = process record)
  *p++ = 0x00; // NDEF length in big endian
  *p++ = 0x00;
  *p++ = 0x00;
  *p++ = ndef_len;
  memcpy(p, ndef, ndef_len); // NDEF payload
  p += ndef_len;
  return p - buf;
}
