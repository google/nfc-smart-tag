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
 * Tests for SHA1 hash. Test data is from the spec.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../crypto/avr_sha1.h"

#include "test.h"

#include "../peripheral/lcd.h"

static void test_example1() {
  test("example1");
  char *source = "abc";
  uint8_t resultarray[20] = {
      0xA9, 0x99, 0x3E ,0x36 ,0x47 ,0x06 ,0x81 ,0x6A,
      0xBA, 0x3E, 0x25 ,0x71 ,0x78 ,0x50 ,0xC2 ,0x6C,
      0x9C ,0xD0 ,0xD8 ,0x9D };

  sha1_hash_t hash;
  sha1(hash, (uint8_t *)source, strlen(source));
  assert(memcmp(hash, resultarray, sizeof(resultarray)) == 0);
}

static void test_example2() {
  test("example2");
  char *source = "abcdbcdecdefdefgefghfghighijhi"
                 "jkijkljklmklmnlmnomnopnopq";
  uint8_t resultarray[20] = {
      0x84, 0x98, 0x3E, 0x44, 0x1C, 0x3B, 0xD2, 0x6E,
      0xBA, 0xAE, 0x4A, 0xA1, 0xF9, 0x51, 0x29, 0xE5,
      0xE5, 0x46, 0x70, 0xF1 };

  sha1_hash_t hash;
  sha1(hash, (uint8_t *)source, strlen(source));
  assert(memcmp(hash, resultarray, sizeof(resultarray)) == 0);
}

static void test_station_data() {
  test("station data");
  uint8_t source[] = {
      0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a ,
      0x01, 0x00, 0x00, 0x00,
      0xf1, 0x1a, 0x00, 0x02, 0x7e, 0x0c, 0xbb, 0x0f,
      0x10, 0x00, 0x20, 0x01, 0x28, 0x00, 0x08, 0x00, 0x18, 0x00, 0x30, 0x3e
  };
  uint8_t resultarray[8] = { // Station only uses first 64bits
      0xb2, 0xe0, 0x42, 0xa1, 0x7b, 0x0d, 0x5d, 0x4c
  };
  sha1_hash_t hash;
  sha1(hash, source, sizeof(source));

  lcd_print_hex(0, hash, 8);

  assert(memcmp(hash, resultarray, sizeof(resultarray)) == 0);
}

void avr_sha1_test(void) {
  test_example1();
  test_example2();
  test_station_data();
}
