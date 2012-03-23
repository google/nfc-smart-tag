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
 * Tests for Felica Push logic.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../nfc/felica_push.h"

#include "../peripheral/lcd.h"

#include "test.h"

static char *url;
uint8_t idm[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

// Adapter method to pass URL making. Set variable url before!
uint8_t get_url(uint8_t *buf, uint8_t __attribute__((unused)) buf_size,
                __attribute__((unused)) void* extra) {
  if (url) {
    strcpy((char *)buf, url);
    return strlen(url);
  } else {
    return 0;
  }
}

static void test_felica_push() {
  test("felica_push");
  uint8_t buf[64];
  uint8_t result;

  char *label = "def";
  url = "abc";

  uint8_t expected[] = {
    0x19, // Total length, including length byte
    0xb0, // command
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // idm
    0x0e, // data size
    0x01, // num data blocks
    0x02, // browser target
    0x08, // parameter size (little endian)
    0x00,
    0x03, // URL size (little endian)
    0x00,
    0x61, // 'abc' (url)
    0x62,
    0x63,
    0x64, // 'def' (label)
    0x65,
    0x66,
    0xfd, // checksum (big endian)
    0x9d,
  };
  result = felica_push_url(buf, sizeof(buf), idm, get_url, NULL, label);

  assert_msg(result == sizeof(expected), "length");
  lcd_print_hex(0, buf+9, 8);
  assert_msg(memcmp(buf, expected, sizeof(expected)) == 0, "data");
}

static void test_felica_push_no_label() {
  test("felica_push_no_label");
  uint8_t buf[64];
  uint8_t result;

  char *label = "";
  url = "abc";

  uint8_t expected[] = {
    0x16, // Total length, including length byte
    0xb0, // command
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, // idm
    0x0b, // data size
    0x01, // num data blocks
    0x02, // browser target
    0x05, // parameter size (little endian)
    0x00,
    0x03, // URL size (little endian)
    0x00,
    0x61, // 'abc' (url)
    0x62,
    0x63,
    0xfe, // checksum (big endian)
    0xcf
  };
  result = felica_push_url(buf, sizeof(buf), idm, get_url, NULL, label);

  assert_msg(result == sizeof(expected), "length");
  lcd_print_hex(0, buf+9, 8);
  assert_msg(memcmp(buf, expected, sizeof(expected)) == 0, "data");
}

static void test_url_error_returns_zero() {
  test("url_error_returns_zero");
  uint8_t buf[64];
  uint8_t result;

  char *label = "";
  url = NULL;  // return error

  result = felica_push_url(buf, sizeof(buf), idm, get_url, NULL, label);
  assert(result == 0);
}

// all tests
void felica_push_test(void) {
  test_felica_push();
  test_felica_push_no_label();
  test_url_error_returns_zero();
}
