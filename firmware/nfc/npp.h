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
 *
 * Support for NPP (NDEF Push Protocol).
 *
 * See source.android.com/compatibility/ndef-push-protocol.pdf
 */

#ifndef NFC_NPP_H_
#define NFC_NPP_H_

#include <stdint.h>

#include "avr/pgmspace.h"

// Fills a buffer with the LLCP service name for NPP, including \0.
prog_char *get_npp_service_name(void);

// Creates an NPP command from a single NDEF record.
uint8_t npp(uint8_t *buf, uint8_t *ndef, uint8_t ndef_len);

#endif  // NFC_NPP_H_
