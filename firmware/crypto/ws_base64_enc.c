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
 * Web safe base64 encoding.
 */

#include "ws_base64_enc.h"

/*
 * Represent 6bits data as a safe ASCII character.
 */
static char __attribute__((noinline))
number2ascii(uint8_t num)
{
  if (num < 26)
    return num + 'A';
  else if (num < 52)
    return (num - 26) + 'a';
  else if (num < 62)
    return (num - 52) + '0';
  else if (num == 62)
    return '-';
  else
    return '_';
}

/*
 * Encodes a data block in web safe base 64 encoding.
 * Returns true on success, false if the output buffer size is
 * insufficient.
 *
 * output: Output buffer receiving the encoded string, null terminated
 * output_size: Size of the output buffer in bytes
 * input: Input buffer
 * input_size: Length of the input buffer in bytes
 */
bool
websafe_base64_encode(char output[], int output_size,
                      const uint8_t input[], int input_size)
{
  int i, j;
  uint8_t pos;
  uint8_t cur, rem;

  pos = cur = rem = 0;
  for (i = 0, j = -1; i < input_size; i++) {
    switch (pos) {
      case 0:
        if (j >= output_size - 1) /* exceed size */
          return false;
        j++;
        cur = (input[i] >> 2);
        rem = (input[i] & 0x03) << 4;
        break;
      case 1:
        cur = rem | (input[i] >> 4);
        rem = (input[i] & 0x0f) << 2;
        break;
      case 2:
        cur = rem | (input[i] >> 6);
        rem = input[i] & 0x3f;
        break;
    }
    if (j >= output_size - 1) /* exceed size */
      return false;
    output[j++] = number2ascii(cur);
    output[j] = number2ascii(rem);
    pos = (pos == 2) ? 0 : (pos + 1);
  }

  if (j >= output_size - 1) /* exceed size */
    return false;
  output[++j] = '\0';

  return true;
}
