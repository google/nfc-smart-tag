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
 * Routines to Communicate with RC-S801/RC-S802 Felica Plug
 *
 * http://www.sony.net/Products/felica/business/tech-support
 */

#include <stdbool.h>
#include <string.h>

#include <util/delay.h>

#include "../nfc/type3tag.h"
#include "../peripheral/lcd.h"
#include "../peripheral/three_wire.h"

#include "rcs801.h"

#define MODE_TYPE3 0x1b
#define PMM_READ   0xC7  // (8 + 1 * #blocks) * 4 ^ 3 / (13.56M / 256 / 16) 
#define PMM_WRITE  0xC7  // (8 + 1 * #blocks) * 4 ^ 3 / (13.56M / 256 / 16)

static const prog_char __init_cmd[] = {
    MODE_TYPE3, 
    PMM_READ,
    PMM_WRITE,
    0x12,  // Data Format Code (IDM[2-3])
    0x34,
    0x01,  // User defined (IDM[4-7]
    0x02,
    0x03,
    0x04
    };

void rcs801_init(void)
{
  twspi_begin_send();
  twspi_send_buf_p(__init_cmd, sizeof(__init_cmd));
  twspi_begin_send();
}

/*
 * Read 2 byte or 3 byte block number (little endian)
 */
static uint16_t __read_block_number() 
{
  uint8_t b;
  
  b = twspi_get();
  if (b & 0x80) {
    return twspi_get();
  } else {
    b = twspi_get();
    return b | ((uint16_t)twspi_get() << 8);
  }
}

/*
 * Processes a command received from the Felica Plug.
 * Handles only Read Without Encryption. Returns an
 * Attribute block or the appropriate segment of an NDEF record.
 */
void rcs801_process_command(uint8_t ndef[], uint16_t ndef_len, bool *has_read_all)
{
  uint8_t cmd;
  uint8_t num_blocks;
  uint8_t block_data[64]; // max 4 blocks
  uint8_t *data = block_data;

  cmd = twspi_get();
  if (cmd == FELICA_READ_WITHOUT_ENCRYPTION) {
    uint8_t i;

    num_blocks = twspi_get();
    if (num_blocks > TYPE3_MAX_NUM_BLOCKS) {
      num_blocks = TYPE3_MAX_NUM_BLOCKS;
    }

    for (i = 0; i < num_blocks; i++) {
      uint16_t block_num = __read_block_number();
      lcd_printf(1, "Felica RD %i", block_num); 
      if (block_num == 0) {
        attribute_block(data, ndef_len);
      }
      if (block_num > NUM_BLOCKS(ndef_len)) {
        memset(data, 0, BLOCK_SIZE);
      } else {
        uint8_t offset = NUM_BYTES(block_num - 1);
        uint8_t num_bytes;        
        if (block_num == NUM_BLOCKS(ndef_len)) {
          num_bytes = ndef_len - offset;
          *has_read_all = true;
          // Extra bytes in last block are ignored, but we wipe to not
          // leak any data. Just fill whole block to reduce code size.
          memset(data, 0x00, BLOCK_SIZE);
        } else {
          num_bytes = BLOCK_SIZE;
        }
        memcpy(data, &ndef[offset], num_bytes);
      }
      data += BLOCK_SIZE;
    }

    twspi_begin_send();
    twspi_send(0x00);  // status
    twspi_send(0x00);
    twspi_send_buf(block_data, num_blocks * 16);
    twspi_end_send();
  }
}

