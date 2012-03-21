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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <avr/io.h>

#include "test.h"

// No need to make header files for these methods
void avr_aes_enc_test(void);
void avr_sha1_test(void);
void ws_base_64_enc_test(void);

void felica_push_test(void);
void llcp_test(void);

int main() {
  test_init();

  avr_aes_enc_test();
  avr_sha1_test();
  ws_base_64_enc_test();

  felica_push_test();
  llcp_test();

  success();
  return 0;  // unreachable
}
