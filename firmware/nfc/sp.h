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

#ifndef NFC_SP_H__
#define NFC_SP_H__

#include <stdbool.h>
#include <stdint.h>

// Function to supply the URL.
typedef bool (*make_url_fp) (uint8_t *buf, uint8_t buf_size, void* extra);

// Generate a smart poster record in the supplied buffer based on the URL
// provided by the make_url function.
uint8_t smart_poster(uint8_t *buf, uint8_t buf_size, const char *label,
                     make_url_fp make_url, void *extra);

#endif // NFC_SP_H_
