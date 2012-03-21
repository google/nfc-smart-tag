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
 * A convenience wrapper for encryption functions.
 */

#ifndef __ENC_H__
#define __ENC_H__

#include <stdint.h>

#define BLOCK_SIZE 16 /* bytes */
#define HASH_SIZE 8 /* bytes */

// Encrypt 128 bit buffer with the key.
void enc128(uint8_t buffer[], uint8_t key[]);

// Encode a buffer by 16 byte blocks, using CTR (Counter) method
void enc128_ctr(uint8_t buffer[], uint8_t buffer_length,
                uint8_t key[], uint8_t counter[]);

// Compute a 64bit hash from a byte buffer.
void hash64(uint8_t result64[], uint8_t buffer[], uint8_t size);

#endif /* __ENC_H__ */
