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
 * Very basic LLCP implementation. Only supports a single data packet, i.e.
 * no sequence numbers, retry etc.
 */

#ifndef NFC_LLCP_H_
#define NFC_LLCP_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <avr/pgmspace.h>

// LLCP PDU Type Values
#define PDU_SYMM    0x00
#define PDU_PAX     0x01
#define PDU_CONNECT 0x04
#define PDU_DISC    0x05
#define PDU_CC      0x06
#define PDU_DM      0x07
#define PDU_I       0x0c
#define PDU_RR      0x0d

// Service Access Point Values
// http://www.nfc-forum.org/specs/nfc_forum_assigned_numbers_register
 #define DSAP_DISC 0x01 // Service discovery
 #define DSAP_SNEP 0x04 // SNEP

// The LLCP Conversation state
enum llcp_state {
  LLCP_INIT,          // Sending CONN
  LLCP_CONN_PENDING,  // Sent CONN, waiting for CC
  LLCP_CONNECTED,     // Sent I, waiting for I
  LLCP_CONFIRMED,     // Received I
  LLCP_DISCONNECTING, // Sent DISC
  LLCP_REJECT,        // Connection request was rejected
  LLCP_DONE
};

typedef struct llcp_ctx_t {
  enum llcp_state state;   // Current state of conversation
  uint8_t dsap;            // Peer's service access point number
  prog_char *service_name; // Service name for lookup
} llcp_ctx;

// Copies ATR_RES byte sequence indicating LLCP capable target to buffer.
uint8_t llcp_atr_res_general_bytes(uint8_t *buffer);

// Returns true iff the buffer contains the LLCP magic bytes.
bool is_llcp_atr_req(uint8_t *buffer);

// Initialize the LLCP conversation state for well-known service.
void llcp_init_wellknown(llcp_ctx *context, uint8_t sap);

// Initialize the LLCP conversation state for service name.
void llcp_init_name(llcp_ctx *context, prog_char *service_name);

// Returns the number of bytes in the llcp header. Payload starts after this.
uint8_t llcp_header_len(uint8_t *buf);

// Determine next LLCP command based on state and last response from peer
uint8_t get_llcp_command(uint8_t *cmd, uint8_t *resp, llcp_ctx *context);

#endif  // NFC_LLCP_H_
