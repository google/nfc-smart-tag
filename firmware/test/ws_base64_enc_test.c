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
 * Very rudimentary unit testing running on the target chip.
 * Results are displayed on the LCD connected to JP1.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../crypto/ws_base64_enc.h"

#include "test.h"

// used to detect memory overwrite
#define MARKER 0xaa

static void test_no_data_makes_empty_string() {
  test("test_no_data_makes_empty_string");
  char out[64];

  memset(out, MARKER, sizeof(out));
  assert(websafe_base64_encode(out, sizeof(out), NULL, 0));
  assert(out[0] == '\0');
  assert(out[1] == MARKER);
}

static void test_buffer_overflow_returns_false() {
  test("test_buffer_overflow_returns_false");
  char out[4];

  memset(out, MARKER, sizeof(out));
  uint8_t in[] = { 0x12, 0x34, 0x56 };

  assert(!websafe_base64_encode(out, 3, in, sizeof(in)));
  assert(out[3] == MARKER);
}

static void test_buffer_cannot_hold_nul_returns_false() {
  test("test_buffer_cannot_hold_nul_returns_false");
  char out[5];

  memset(out, MARKER, sizeof(out));
  uint8_t in[] = { 0x12, 0x34, 0x56 };

  websafe_base64_encode(out, 4, in, sizeof(in));
  assert(out[4] == MARKER);
}


static void test_empty_buffer_writes_nothing() {
  test("test_empty_buffer_writes_nothing");
  char out[1];

  memset(out, MARKER, sizeof(out));
  uint8_t in[] = { 0x01 };

  assert(!websafe_base64_encode(out, 0, in, sizeof(in)));
  assert(out[0] == MARKER);
}

static void test_simple_case() {
  test("test_simple_case");
  char out[6]; // one extra to test for overflow

  memset(out, MARKER, sizeof(out));
  uint8_t input[] = { 0x12, 0x34, 0x56 };
  assert(websafe_base64_encode(out, 5, input , sizeof(input)));
  assert(out[0] == 'E');
  assert(out[1] == 'j');
  assert(out[2] == 'R');
  assert(out[3] == 'W');
  assert(out[4] == 0x00);
  assert(out[5] == MARKER);
}

void ws_base_64_enc_test(void) {
  test_no_data_makes_empty_string();
  test_buffer_overflow_returns_false();
  test_buffer_cannot_hold_nul_returns_false();
  test_empty_buffer_writes_nothing();
  test_simple_case();
}
