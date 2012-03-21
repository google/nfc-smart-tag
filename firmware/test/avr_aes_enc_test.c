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
 * Tests for AES 128encryption. Test data is taken from the spec:
 * http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../crypto/avr_aes_enc.h"

#include "test.h"

#include "../peripheral/lcd.h"
#include "../peripheral/timer.h"

static void test_vector_spec() {
  test("vector_spec");
  aes128_ctx_t ctx;
  uint8_t plaintext[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                          0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
  uint8_t key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
  uint8_t expected[] = { 0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30,
                       0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a };
  uint8_t buffer[16];

  start_timer(TIMER_RES_CLOCK);  
  aes128_init(key, &ctx);
  stop_timer();
  lcd_printf(0, "aes key %i", get_timer());
  memcpy(buffer, plaintext, 16);
  start_timer(TIMER_RES_CLOCK);  
  aes128_enc(buffer, &ctx);  
  stop_timer();
  lcd_printf(0, "aes enc %i", get_timer());
  assert(memcmp(buffer, expected, 16) == 0);
}

static void test_vector_gladman() {
  test("vector_gladman");
  aes128_ctx_t ctx;
  uint8_t plaintext[] = { 0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
                          0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34 };
  uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 
                    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
  uint8_t expected [] = { 0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 
                          0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32 };
  uint8_t buffer[16];
  
  aes128_init(key, &ctx);
  memcpy(buffer, plaintext, 16);
  aes128_enc(buffer, &ctx);  
  assert(memcmp(buffer, expected, 16) == 0);
}

void avr_aes_enc_test(void) {
  test_vector_spec();
  test_vector_gladman();
}
