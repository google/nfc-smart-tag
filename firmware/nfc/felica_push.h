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
 *  Data format to push a URL to a Felica handset.
 *
 * See http://www.felicanetworks.co.jp/tech/ for specification:
 * - Mobile Felica IC Chip Function Manual
 * - External Reader/Writer Data Format Specification
 */

#ifndef NFC_FELICA_PUSH_H_
#define NFC_FELICA_PUSH_H_

#include <stdint.h>

#define IDM_LENGTH 8

// Function to supply the URL. Returns number of bytes written to buf.
typedef uint8_t (*make_url_fp) (uint8_t *buf, uint8_t buf_size, void* extra);

// Creates a Felica Push command in the specified buffer.
uint8_t felica_push_url(
    uint8_t *buf, uint8_t buf_size,
    uint8_t *idm, make_url_fp make_url, void * extra,
    const char *label);

#endif  // NFC_FELICA_PUSH_H_
