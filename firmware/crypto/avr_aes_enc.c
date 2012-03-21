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
 * http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
 * AES-128 Encryption based on a naive implementation from the spec:
 * and then optimized as described in the comments.
 *
 * Key expansion: ~4000 cycles (1.2ms @ 3.58MHz)
 * Encrypting one block: ~8000 cycles (2.2ms @ 3.58MHz)
 *
 * Method names use a different convention as they are taken from the spec.
 */

#include <stdint.h>
#include <string.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "avr_aes_enc.h"

const uint8_t PROGMEM sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
    0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
    0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
    0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
    0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
    0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
    0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
    0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
    0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
    0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
    0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
    0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

#define BPOLY 0x1b // Lower 8 bits of (x^8+x^4+x^3+x+1), ie. (x^4+x^3+x+1).
#define KEYLENGTH 16

/*
 * Substitute bytes.
 *
 * Optimization:
 * - Use precomputed sbox
 */
static void SubBytes(uint8_t *data, uint8_t count)
{
  do {
    *data++ = pgm_read_byte(sbox + *data);
  } while (--count);
}

/*
 * Shift rows 1 -3 of state as per spec.
 */
static void ShiftRows(uint8_t *state)
{
  uint8_t tmp;
  // row 1
  tmp = state[1];
  state[1] = state[5];
  state[5] = state[9];
  state[9] = state[13];
  state[13] = tmp;
  // row 2
  tmp = state[2];
  state[2] = state [10];
  state[10] = tmp;
  tmp = state[6];
  state[6] = state[14];
  state[14] = tmp;
  // row 3
  tmp = state[15];
  state[15] = state[11];
  state[11] = state[7];
  state[7] = state[3];
  state[3] = tmp;
}

/*
 * Multiply by 2 modulo 0x1b (see Spec 4.2.1)
 * return (num << 1) ^ (num & 0x80 ? BPOLY : 0);
 *
 * Optimizations:
 * - Rewritten in assembly using carry bit
 */
static uint8_t xtime(uint8_t num)
{
  asm volatile(
      "lsl %0\n\t"
      "brcc L_%=\n\t"
      "ldi r24, lo8(27)\n\t"
      "eor %0, r24\n\t"
      "L_%=:\n\t"
      : "=r" (num) : "0" (num) : "r24");
  return num;
}

/*
 *  Take dot products of each matrix row and the column vector.
 *  02 03 01 01
 *  01 02 03 01
 *  01 01 02 03
 *  03 01 01 02
 *
 * Optimizations:
 * - Multiply by 2 using xtime
 * - Implement mul(3,x) as mul(2,x) ^ x
 * - Precompute xor for full column and undo the one not needed
 * - Replace (mul(2,x) + mul(2,y)) with mul(2, x+y)
 */
static void MixColumn(uint8_t *column)
{
  uint8_t result[4];
  uint8_t sum = column[0] ^ column[1] ^ column[2] ^ column[3];

  result[0] = xtime(column[0] ^ column[1]);
  result[0] ^= sum ^ column[0];

  result[1] = xtime(column[1] ^ column[2]);
  result[1] ^= sum ^ column[1];

  result[2] = xtime(column[2] ^ column[3]);
  result[2] ^= sum ^ column[2];

  result[3] = xtime(column[0] ^ column[3]);
  result[3] ^= sum ^ column[3];

  // Copy temporary result to original column.
  column[0] = result[0];
  column[1] = result[1];
  column[2] = result[2];
  column[3] = result[3];
}

static void MixColumns(uint8_t *state)
{
  MixColumn(state + 0*4);
  MixColumn(state + 1*4);
  MixColumn(state + 2*4);
  MixColumn(state + 3*4);
}

static void XORBytes(uint8_t *bytes1, uint8_t *bytes2, uint8_t count)
{
  do {
    *bytes1++ ^= *bytes2++;
  } while(--count);
}

/*
 * Encodes one 16 byte (128bit) block of data in place.
 */
void aes128_enc(uint8_t * block, aes128_ctx_t *ctx)
{
  uint8_t round = AES128_ROUNDS - 1;
  uint8_t *expandedKey = ctx->expanded_key;

  XORBytes(block, expandedKey, 16);  // AddRoundKey
  expandedKey += BLOCKSIZE;

  do {
    SubBytes(block, 16);
    ShiftRows(block);
    MixColumns(block);
    XORBytes(block, expandedKey, 16);  // AddRoundKey
    expandedKey += BLOCKSIZE;
  } while(--round);

  SubBytes(block, 16);
  ShiftRows(block);
  XORBytes(block, expandedKey, 16);  // AddRoundkey
}

static void RotWord(uint8_t *word)
{
  uint8_t temp = word[0];
  word[0] = word[1];
  word[1] = word[2];
  word[2] = word[3];
  word[3] = temp;
}

/*
 * Key expansion from 16 bytes to 176 bytes.
 *
 * Optimizations:
 * - Compute rcon on the fly
 * - Only consider lowest byte of rcon as others are 0
 * - Avoid MOD operation
 * - Only consider 128bit key
 */
static void KeyExpansion(uint8_t *key, uint8_t *expandedKey)
{
  uint8_t temp[4];
  uint8_t i;
  uint8_t next_i;
  uint8_t rcon = 0x01; // Round constant.

  // Copy key to start of expanded key.
  memcpy(expandedKey, key, KEYLENGTH);

  // Prepare last 4 bytes in temp.
  expandedKey += KEYLENGTH - 4;
  temp[0] = *(expandedKey++);
  temp[1] = *(expandedKey++);
  temp[2] = *(expandedKey++);
  temp[3] = *(expandedKey++);

  i = KEYLENGTH;
  next_i = KEYLENGTH;
  while (i < BLOCKSIZE*(AES128_ROUNDS+1)) {
    // Are we at the start of the next key size?
    if (i == next_i) {
      // temp still contains last block
      RotWord(temp);
      SubBytes(temp, 4);
      temp[0] ^= rcon;
      rcon = xtime(rcon);
      next_i += KEYLENGTH;
    }
    XORBytes(temp, expandedKey - KEYLENGTH, 4);
    *(expandedKey++) = temp[0];
    *(expandedKey++) = temp[1];
    *(expandedKey++) = temp[2];
    *(expandedKey++) = temp[3];
    i += 4;
  }
}

/*
 * Expands the 16 byte (128bit) key into the context.
 */
void aes128_init(uint8_t *key, aes128_ctx_t *ctx)
{
  KeyExpansion(key, ctx->expanded_key);
}
