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
 * Generate a smart poster binary image.
 *
 * See http://www.nfc-forum.org/specs/spec_list/ for specification.
 */

#include <string.h>

#include "sp.h"

/**
 * Populates a buffer with a NDEF Smart Poster record.
 * If a label is provided (not NULL), includes a Title record.
 *
 * Based on the following specifications:
 * NFC Data Exchange Format (NDEF)
 * URI Record Type Definition
 * Text Record Type Definition
 *
 * http://www.nfc-forum.org/specs/spec_list/
 *
 * Arguments:
 * buf: the buffer to hold the Smart Poster binary image
 * buf_size: Size of the available buffer
 * label: optional label to include as Title record
 * make_url: pointer to function that supplies the URL
 * extra: any extra parameters to be passed to make_url
 *
 * Returns:
 * Number of bytes written to buffer, 0 zero on error
 */
uint8_t
smart_poster(uint8_t *buf, uint8_t buf_size, const char *label,
            make_url_fp make_url, void *extra)
{
  uint8_t idx, len_idx, sp_head, url_len_idx;
  uint8_t url_len;

  // Min size needed for a 1 character URL
  if (buf_size < 10) {
    return 0;
  }
  idx = 0;
  /*
   * 0xd1 =
   * MB (Message Begin) + ME (Message End) + SR (Short Record) +
   * TNF = 1 (Well Known).  See "NFC Data Exchange Format (NDEF)".
   */
  buf[idx++] = 0xd1;
  buf[idx++] = 0x02; // record name length ('Sp')
  len_idx = idx++; // will be filled later.
  buf[idx++] = 'S'; // Smart Poster Type.
  buf[idx++] = 'p';
  sp_head = idx;
  if (label != NULL) {
    uint8_t txt_len_idx, text_head;
    uint8_t label_length = strlen(label);
    buf[idx++] = 0x91; // MB=1 ME=0 CF=0 SR=1 IL=0 TNF=001
    buf[idx++] = 0x01; // Type Length = 1
    txt_len_idx = idx++; // Filled below.
    buf[idx++] = 'T'; // Text Record
    text_head = idx;
    buf[idx++] = 0x02; // UTF-8, Language code length
    buf[idx++] = 'e'; // ISO language code
    buf[idx++] = 'n';
    memcpy(&buf[idx], label, label_length);
    idx += label_length;
    buf[txt_len_idx] = idx - text_head;
    // beginning of 'U' record.
    buf[idx++] = 0x51; // MB=0 ME=1 CF=0 SR=1 IL=0 TNF=001
  } else {
    // beginning of 'U' record.
    buf[idx++] = 0xd1; // MB=1 ME=1 CF=0 SR=1 IL=0 TNF=001
  }
  buf[idx++] = 0x01; // Record name length.
  url_len_idx = idx++; // Filled below.
  buf[idx++] = 'U'; // URL type.  See "URI Record Type Definition".
  buf[idx++] = 0x00; // String is literal URL incl protocol.

  // Append URL to be sent.
  if (!((*make_url)(&buf[idx], buf_size - idx, extra))) {
    return 0;
  }
  url_len = strlen((char*)&buf[idx]);
  idx += url_len;

  // Fill in length bytes
  buf[len_idx] = idx - sp_head;
  buf[url_len_idx] = url_len + 1; // sizeof 'U'

  return idx;
}
