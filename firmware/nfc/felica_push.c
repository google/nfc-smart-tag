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
 * Data format to push a URL to a Felica handset.
 *
 * See http://www.felicanetworks.co.jp/tech/ for specification:
 * - Mobile Felica IC Chip Function Manual
 * - External Reader/Writer Data Format Specification
 */

#include <stdint.h>
#include <string.h>

#include "felica_push.h"

#define L8(x) ((x) & 0xff)
#define H8(x) (((x) >> 8) & 0xff)

/*
 * Computes 16 bit checksum over max 256 bytes so that all data added
 * to checksum yields 0.
 */
static uint16_t __checksum(uint8_t *data, uint8_t len)
{
  uint16_t cksum = 0;
  do {
    cksum -= *data++;
  } while (--len);
  return cksum;
}

/*
 * Creates a Felica Push command in the specified buffer. The payload
 * length is limited to 192 or 224 bytes, depending on Felica OS version.
 *
 * buffer: target buffer where the command will be stored.
 * buffer_size: size of buffer
 * idm: IDm of target device
 * get_url: callback method that supplies the URL
 * extra: data to be passed to get_url
 * label: coupon label, used by KDDI devices only
 * returns: number of bytes written to buffer, 0 on error
 */
uint8_t felica_push_url(
    uint8_t *buf, uint8_t buf_size,
    uint8_t *idm, make_url_fp make_url, void *extra,
    const char *label)
{
  uint8_t url_size, label_size;
  uint8_t len_idx, param_len_idx, block_idx;
  uint8_t idx = 0;
  uint16_t cksum;

  // Command header per Mobile Felica IC Chip Function Manual
  buf[idx++] = 0xb0;
  memcpy(&buf[idx], idm, IDM_LENGTH);
  idx += IDM_LENGTH;
  len_idx = idx++; // filled in below

  // Payload as per External Reader/Writer Data Format Specification
  block_idx = idx;
  buf[idx++] = 0x01; // Number of data blocks
  buf[idx++] = 0x02; // Target: Browser
  param_len_idx = idx; // filled in below
  idx += 2;

  // Fill in URL and size via callback
  url_size = (*make_url)(&buf[idx+2], buf_size - idx, extra);
  if (url_size == 0) {
    return 0;
  }
  buf[idx++] = L8(url_size);
  buf[idx++] = H8(url_size);
  idx += url_size;

  // Fill in label
  label_size = strlen(label);
  memcpy(&buf[idx], label, label_size);
  idx += label_size;

  // Set parameter size
  buf[param_len_idx] = L8(idx - param_len_idx - 2);
  buf[param_len_idx + 1] = H8(idx - param_len_idx - 2);

  // Set Checksum of whole block
  cksum = __checksum(&buf[block_idx], idx - block_idx);
  buf[idx++] = H8(cksum);
  buf[idx++] = L8(cksum);

  // Fill in command size
  buf[len_idx] = idx - len_idx - 1;

  return idx;
}
