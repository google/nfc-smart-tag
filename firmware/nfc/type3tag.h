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
 * Emulates a NFC Forum Type 3 Tag.
 *
 * Based on the following specifications:
 * NFC Digital Protocol Technical Specification
 *   - Chapter 6: NFC_F Technology
 *   - Chapter 10 Type 3 Tag Platform
 *
 * Type 3 Tag Operation Technical Specification
 *
 * http://www.nfc-forum.org/specs/spec_list/
 */

#ifndef NFC_TYPE3TAG_H_
#define NFC_TYPE3TAG_H_

#include <stdbool.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

// Enough for 4 data blocks of 16 bytes each + header
#define TYPE3_BUFFER_SIZE 100

// Returns the 2 byte syscode for a Type 3 Card
const prog_char *get_card_syscode(void);

// Returns the 8 byte PMM for a Type 3 Card
const prog_char *get_card_pmm(void);

// Determines the response to a Type 3 command received from initiator
uint8_t get_type3_response(
    uint8_t *resp,
    uint8_t *cmd,
    uint8_t card_idm[],
    uint8_t record[], int record_len,
    bool *has_read_all);

#endif  // NFC_TYPE3TAG_H_
