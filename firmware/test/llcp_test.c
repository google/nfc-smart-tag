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
 * Tests for LLCP protocol.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../nfc/llcp.h"

#include "../peripheral/lcd.h"

#include "test.h"

prog_char service_name[] = "abc";

static void test_conn_wellknown() {
  test("conn_wellknown");
  uint8_t len;
  uint8_t cmd[50];
  uint8_t resp[] = { 0x00, 0x00 };  // SYMM
  llcp_ctx context;
  uint8_t expected[] = {
    0x11, // DSAP & PTYPE (CONN)
    0x20, // SSAP
  };

  llcp_init_wellknown(&context, 4);
  len = get_llcp_command(cmd, resp, &context);

  assert_msg(len == sizeof(expected), "length");
  assert_msg(memcmp(cmd, expected, sizeof(expected)) == 0, "data");
  assert_msg(context.state == LLCP_CONN_PENDING, "state");
}

static void test_conn_name() {
  test("conn_name");
  uint8_t len;
  uint8_t cmd[50];
  uint8_t resp[] = { 0x00, 0x00 };  // SYMM
  llcp_ctx context;
  uint8_t expected[] = {
    0x05, // DSAP & PTYPE (CONN)
    0x20, // SSAP
    0x06, // SN Parameter
    0x03, // Length
    0x61, // Requested service name
    0x62,
    0x63
  };

  llcp_init_name(&context, service_name);
  len = get_llcp_command(cmd, resp, &context);

  assert_msg(len == sizeof(expected), "length");
  assert_msg(memcmp(cmd, expected, sizeof(expected)) == 0, "data");
  assert_msg(context.state == LLCP_CONN_PENDING, "state");
}

// all tests
void llcp_test(void) {
  test_conn_wellknown();
  test_conn_name();
}
