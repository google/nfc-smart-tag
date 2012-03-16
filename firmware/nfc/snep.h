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
 * See http://www.nfc-forum.org/specs/spec_list/
 */

#ifndef NFC_SNEP_H_
#define NFC_SNEP_H_

#include <avr/io.h>

// Response packet status codes
#define SNEP_RESP_CONTINUE 0x80
#define SNEP_RESP_SUCCESS 0x81
#define SNEP_RESP_BAD_REQ 0xC2

uint8_t snep_put(uint8_t *buf, uint8_t data_len);

uint8_t snep_response_status(uint8_t *buf);

#endif  // NFC_SNEP_H_
