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

// Compute number of blocks needed to store X bytes. Use shift for efficiency.
#define BLOCK_SIZE 16
#define NUM_BLOCKS(X) (((X) + (BLOCK_SIZE - 1)) >> 4)
#define NUM_BYTES(X) ((X) << 4)

// Enough for 4 data blocks of 16 bytes each + header
#define TYPE3_BUFFER_SIZE 100

// We are able to provide 4 blocks in one read
#define TYPE3_MAX_NUM_BLOCKS 4

#define FELICA_POLL 0x00
#define FELICA_READ_WITHOUT_ENCRYPTION 0x06


// Returns the 2 byte syscode for a Type 3 Card
const prog_char *get_card_syscode(void);

// Returns the 8 byte PMM for a Type 3 Card
const prog_char *get_card_pmm(void);

// Fills a buffer with the attrbute block
uint8_t attribute_block(uint8_t *buf, uint16_t data_len);

// Determines the response to a Type 3 command received from initiator
uint8_t get_type3_response(
    uint8_t *resp,
    uint8_t *cmd,
    uint8_t card_idm[],
    uint8_t record[], uint16_t record_len,
    bool *has_read_all);

#endif  // NFC_TYPE3TAG_H_
