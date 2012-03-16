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
 * AES-128 Encryption based on a naive implementation from the spec:
 * http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
 * and then optimized as described in the comments.
 *
 * Key expansion: ~4000 cycles (1.2ms @ 3.58MHz)
 * Encrypting one block: ~8000 cycles (2.2ms @ 3.58MHz)
 */

#ifndef EXPERIMENTAL_NFC_AVR_BASE_AVR_AES_ENC_H_
#define EXPERIMENTAL_NFC_AVR_BASE_AVR_AES_ENC_H_

#define BLOCKSIZE 16 // Block size in number of bytes.
#define AES128_ROUNDS 10

#include <stdint.h>

typedef struct aes128_ctx {
  uint8_t expanded_key [BLOCKSIZE * (AES128_ROUNDS+1)];
} aes128_ctx_t;

// Expands the 16 byte (128bit) key into the context.
void aes128_init(uint8_t *key, aes128_ctx_t *ctx);

// Encodes one 16 byte (128bit) block of data in place.
void aes128_enc(uint8_t *block, aes128_ctx_t *ctx);

#endif  // EXPERIMENTAL_NFC_AVR_BASE_AVR_AES_ENC_H_
