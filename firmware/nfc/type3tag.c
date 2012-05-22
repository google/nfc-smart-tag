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

#include <string.h>

#include <avr/pgmspace.h>

#include "../peripheral/lcd.h"

#include "type3tag.h"

// Service code for NDEF data on Type 3 Tag
#define NDEF_SERVICE_CODE 0x000b

// System code for NDEF enabled Type3 Tag
static const prog_char card_syscode[] = {0x12, 0xfc};

// Sample card PMm w/ check wait of 2.4ms + 2.4ms/block
// See Section 2.3.1.2 of Type 3 Tag Operation
static const prog_char card_pmm[] = {
    0x01, 0x20, 0x22, 0x04, 0x27, 0x3f, 0x7f, 0xff};

// Extract high and low bytes
#define L8(x) (x & 0xff)
#define H8(x) ((x >> 8) & 0xff)

// Length of IDM block
#define IDM_BYTES 8

// First byte is length, payload starts with next byte.
#define OFS_PAYLOAD OFS_DATA+1

/*
 * Writes the command header for a Type 3 Tag Check (Read without Encryption)
 * response to a buffer.
 *
 * Returns: number of bytes written
 */
static uint8_t
__check_response_header(uint8_t *buf, uint8_t *id, uint8_t num_blocks)
{
  uint8_t head = 0;

  buf[head++] = 0x00; // Length: filled out below.
  buf[head++] = 0x07; // Response to Read Command
  memcpy(&buf[head], id, IDM_BYTES);
  head += IDM_BYTES;
  buf[head++] = 0x00; // stat flag 1 (00: clear)
  buf[head++] = 0x00; // stat flag 2 (00: clear)
  buf[head++] = num_blocks; // # of blocks in reply
  buf[0] = head + NUM_BYTES(num_blocks);
  return head;
}

/**
 * Compute checksum for the attribute info block.
 */
static uint16_t __attr_checksum(uint8_t *b, int length)
{
  int i;
  uint16_t cksum = 0;
  for (i = 0; i < length; i++) {
    cksum += b[i];
  }
  return cksum;
}

/*
 * Populates the 16 byte attribute block for a Type 3 tag as defined in
 * Chapter 6 of NFC Forum Type 3 Tag Operation Technical Specification.
 *
 * Allows up to 4 blocks to be checked(read) at one time to not exceed
 * our working buffer.
 *
 * Returns: number of bytes written (16)
 */
uint8_t attribute_block(uint8_t *buf, uint16_t data_len) {
  uint8_t head = 0;
  uint16_t cksum;

  // 16 bytes data (1 block).
  uint8_t data_pos = head;
  buf[head++] = 0x10; // ver
  buf[head++] = // nbr (# blocks to check)
                (NUM_BLOCKS(data_len) < TYPE3_MAX_NUM_BLOCKS) ?
                 NUM_BLOCKS(data_len) : TYPE3_MAX_NUM_BLOCKS;
  buf[head++] = 0x01; // nbw (# blocks to update)
  buf[head++] = H8(NUM_BLOCKS(data_len)); // # Blocks available
  buf[head++] = L8(NUM_BLOCKS(data_len));
  buf[head++] = 0x00; // unused (5)
  buf[head++] = 0x00; // unused (6)
  buf[head++] = 0x00; // unused (7)
  buf[head++] = 0x00; // unused (8)
  buf[head++] = 0x00; // writeF (00: finished)
  buf[head++] = 0x00; // RW Flag (00: read only)
  buf[head++] = 0x00; // Ln (11) upper
  buf[head++] = H8(data_len); // Ln middle (12)
  buf[head++] = L8(data_len); // Ln lower  (13)
  cksum = __attr_checksum(&buf[data_pos], 14);
  buf[head++] = H8(cksum); // Checksum (14) upper
  buf[head++] = L8(cksum); // Checksum (15) lower
  // end of block.
  return head;
}

/*
 * Writes a SENSF_RES (response to polling command) to a buffer according
 * to NFC Digital Protocol Technical Specification 1.0 Section 6.6.2
 *
 * Returns: the number of bytes written.
 */
static uint8_t
__poll_response(uint8_t *buf, uint8_t *card_idm, bool include_syscode) {
  // 1: Command (0x01)
  // 2-9: NFCID2: idm
  // 10-14: Pad
  // 15: Max Response Time Info (MRTI) Check
  // 16: Max Response Time Info (MTRI) Update
  // 17: Pad
  // 18-19: Response Data (RD): Syscode
  uint8_t head = 0;
  buf[head++] = 0x00; // Length, filled below
  buf[head++] = 0x01; // Command
  memcpy(&buf[head], card_idm, IDM_BYTES);
  head += IDM_BYTES;
  memcpy_P(&buf[head], card_pmm, sizeof(card_pmm));
  head += sizeof(card_pmm);
  if (include_syscode) { // asked to send system code.
    memcpy_P(&buf[head], card_syscode, sizeof(card_syscode));
    head += sizeof(card_syscode);
  }
  buf[0] = head; // Len
  return head;
}

/**
 * Computes a response packet to the following Type 3 Tag command:
 *   SENSF_REQ (Polling): 0x00
 *   Check (Read w/o Encryption) 0x06
 *
 * Arguments:
 *   resp: output buffer for the computed respone
 *   cmd: command received from initiator
 *   card_idm: used to reply to polling command
 *   record: payload, e.g. smart poster NDEF record
 *   record_len: length of the record data
 *   has_read_all: set to true if all data was read by the initiator
 *
 * Returns:
 *   Length of the computed response, 0 on error
 */
uint8_t get_type3_response(
    uint8_t *resp,
    uint8_t *cmd,
    uint8_t card_idm[],
    uint8_t record[], uint16_t record_len,
    bool *has_read_all)
{
  uint8_t resp_len = 0;

  // Respond to Commands
  switch (cmd[0]) {
  case FELICA_POLL: // Polling (SENF_REQ) Command
    // 1: Command (0x00)
    // 2/3: System Code
    // 4: Request Code (RC) 0x01: Include syscode
    // 5: Time Slot Number (TSN)
    lcd_printf(0, "Felica Poll");
    // Respond with SENSF_RES if syscode matches ours or is 0xFFFF (all)
    if ((memcmp_P(&cmd[1], card_syscode, sizeof(card_syscode)) == 0) ||
        (cmd[1] == 0xff && cmd[2] == 0xff)) {
      // Include system code if requested (0x01).
      // We don't hanlde "Advanced Protocol Features" (0x02)
      resp_len = __poll_response(resp, card_idm, cmd[3] == 0x01);
    }
    break;

  case FELICA_READ_WITHOUT_ENCRYPTION: // Read Without Encryption (Check).
    // 0: Command (0x06)
    // 1-8: IDm (8 bytes)
    // 9: Number of Services (usually one)
    // 10,11: Service Code List
    // 12: Number of Blocks requested
    // 13+: Block List (2 bytes each, second byte has block #)
    if ((cmd[9] == 1) &&
        (cmd[10] == L8(NDEF_SERVICE_CODE)) &&
        (cmd[11] == H8(NDEF_SERVICE_CODE)) &&
        (cmd[13] == 0x80)) {
      if (cmd[12] == 1 && cmd[14] == 0x00) {
        // Block 0: Attribute Information Block
        resp_len = __check_response_header(resp, &cmd[1], 1);
        resp_len += attribute_block(&resp[resp_len], record_len);
        lcd_printf(0, "Felica RD Attr");
      } else {
        // Data block: Return requested blocks
        uint8_t req_blocks = cmd[12];
        uint8_t max_blocks = NUM_BLOCKS(record_len);
        uint8_t num_bytes;
        uint8_t b;
        resp_len = __check_response_header(resp, &cmd[1], req_blocks);
        // 16 bytes block data for each block
        for (b = 0; b < req_blocks; b++) {
          uint8_t block = cmd[14 + (b << 1)] - 1;
          if (block >= max_blocks) {
            return false;
          }
          uint8_t offset = NUM_BYTES(block);
          if (block == max_blocks-1) {
            num_bytes = record_len - offset;
            *has_read_all = true;
            // Extra bytes in last block are ignored, but we wipe to not
            // leak any data. Just fill whole block to reduce code size.
            memset(&resp[resp_len], 0x00, BLOCK_SIZE);
          } else {
            num_bytes = BLOCK_SIZE;
          }
          memcpy(&resp[resp_len], &record[offset], num_bytes);
          resp_len += BLOCK_SIZE;
        }
        lcd_printf(0, "Felica RD %i %i", cmd[14]-1, req_blocks);
      }
    } else {
      return false;
    }
    break;

  default:
    lcd_printf(1, "unknwn %02X%02X%02X", cmd[0], cmd[1], cmd[2]);
    return false;
  }
  return resp_len;
}
