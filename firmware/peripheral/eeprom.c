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
 * EEPROM helper routines.
 */

#include <avr/eeprom.h>
#include <avr/io.h>

#include <stdint.h>
#include <string.h>

#include "eeprom.h"


/*
 * Increments a specified uint32_t counter stored in EEPROM, and returns new
 * counter value.
 *
 * Returns: new counter value.
 */
uint32_t increment_eeprom_uint32(uint32_t *pointer_eeprom)
{
  uint8_t digit;
  union {
    uint32_t u32;
    uint8_t u8[sizeof(uint32_t)];
  } counter;

  eeprom_read_block(&counter.u32, pointer_eeprom, sizeof(uint32_t));

  /*
   * Save unnecessary writes. Higher bytes rarely change, so we can
   * save time and reduce EEPROM wear by not writing them.
   * Note: this code assumes little endian.
   */
  digit = 0;
  for (digit = 0; digit < sizeof(uint32_t) ;) {
    if (++counter.u8[digit++] != 0)
      break;
  }
  /* Write only as many bytes as actually changed */
  eeprom_write_block(&counter.u32, (uint8_t*)pointer_eeprom, digit);
  return counter.u32;
}
