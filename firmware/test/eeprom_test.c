/*
 * Copyright 2011 Google Inc. All Rights Reserved.
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
 * Test routines for EEPROM word increment.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <avr/eeprom.h>

#include "../peripheral/eeprom.h"

#include "test.h"

static void test_simple_increment() {
  test("test_simple_increment");
  uint32_t *memory = (uint32_t *)100;

  eeprom_write_dword(memory, 1);
  uint32_t result = increment_eeprom_uint32(memory);
  assert(result == 2);
  assert(eeprom_read_dword(memory) == 2);
}

static void test_byte_overflow() {
  test("test_byte_overflow");
  uint32_t *memory = (uint32_t *)104;

  eeprom_write_dword(memory, 0xFF);
  uint32_t result = increment_eeprom_uint32(memory);
  assert(result == 0x100);
  assert(eeprom_read_dword(memory) == 0x100);
}

static void test_overflow_into_4th_byte() {
  test("test_overflow_into_4th_byte");
  uint32_t *memory = (uint32_t *)108;

  eeprom_write_dword(memory, 0x00FFFFFF);
  uint32_t result = increment_eeprom_uint32(memory);
  assert(result == 0x01000000);
  assert(eeprom_read_dword(memory) == 0x01000000);
}

static void test_rollover_to_zero() {
  test("test_rollover_to_zero");
  uint32_t *memory = (uint32_t *)112;

  eeprom_write_dword(memory, 0xFFFFFFFF);
  eeprom_write_byte((uint8_t *)memory + 4, 0xa5);
  uint32_t result = increment_eeprom_uint32(memory);
  assert(result == 0x00000000);
  assert(eeprom_read_dword(memory) == 0x00000000);
  assert(eeprom_read_byte((uint8_t *)memory + 4) == 0xa5);
}

void eeprom_test(void) {
  test_simple_increment();
  test_byte_overflow();
  test_overflow_into_4th_byte();
  test_rollover_to_zero();
}
