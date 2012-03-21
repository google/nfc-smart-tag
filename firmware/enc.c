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
 * A convenience wrapper for encryption functions. Adjust this code
 * if you chose a different AES library or use a different cipher.
 */

#include <string.h>

#include "crypto/avr_aes_enc.h"
#include "crypto/avr_sha1.h"

#include "enc.h"

/*
 * Encode a 128bit buffer with the key.
 */
void enc128(uint8_t buffer[], uint8_t key[])
{
  aes128_ctx_t ctx;

  aes128_init(key, &ctx);
  aes128_enc(buffer, &ctx);
}

/*
 * Encode a buffer by 128 bit blocks, using CTR (Counter) method.
 *
 * Arguments:
 * buffer: Plain text to be encrypted. Overwritten with cipher text.
 * buffer_length: Length of buffer in bytes.
 * key: Encryption key.
 * counter: 128 bit counter for encryption. Modified by this method.
 */
void enc128_ctr(uint8_t buffer[], uint8_t buffer_length,
                uint8_t key[], uint8_t counter[])
{
  uint8_t tmp[16];

  aes128_ctx_t ctx;
  aes128_init(key, &ctx);

  for (int i = 0; i < buffer_length; i += BLOCK_SIZE) {
    uint8_t *block = &buffer[i];
    uint8_t remain = buffer_length - i;
    uint8_t j, count;

    memcpy(tmp, counter, sizeof(tmp));
    aes128_enc(tmp, &ctx);
    count = (remain < BLOCK_SIZE) ? remain : BLOCK_SIZE;
    for (j = 0; j < count; j++) {
      block[j] ^= tmp[j];
    }
    /* Notice: will not overflow since buffer is always smaller than 256*16 */
    counter[15]++;
  }
}

/*
 * Compute a 64bit hash from a byte buffer.
 */
void hash64(uint8_t result64[], uint8_t buffer[], uint8_t size)
{
  sha1_hash_t dest;

  sha1(dest, buffer, size);
  memcpy(result64, dest, HASH_SIZE);
}
