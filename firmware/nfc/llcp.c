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
 * no sequence numbers, retry etc. Developed based on the NFC Forum Spec:
 * http://www.nfc-forum.org/specs/spec_list/
 */

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "../peripheral/lcd.h"

#include "llcp.h"

#define SSAP 0x20 // Local service point

// LLCP ATR_RES general bytes
static uint8_t PROGMEM general_bytes[] = {
  0x46, 0x66, 0x6D, // LLCP Magic number
  0x01, 0x01, 0x10, // TLV Version 1.0
  0x03, 0x02, 0x00, 0x13, // TLV WKS
  0x04, 0x01, 0x96 // TLV Link Timeout 150 x 10ms = 1.5s
};

/*
 * Copies ATR_RES byte sequence indicating LLCP capable target to buffer
 * and returns number of bytes written. Buffer size should be 48 bytes
 * according to spec.
 */
uint8_t llcp_atr_res_general_bytes(uint8_t *buffer)
{
  memcpy_P(buffer, general_bytes, sizeof(general_bytes));
  return (uint8_t)sizeof(general_bytes);
}

/*
 * Returns true iff the buffer contains the LLCP magic bytes.
 */
inline bool is_llcp_atr_req(uint8_t *buffer)
{
  return buffer[0] == 0x46 && buffer[1] == 0x66 && buffer[2] == 0x6D;
}

/*
 * Initialize the LLCP conversation state.
 */
void llcp_init_wellknown(llcp_ctx *context, uint8_t sap)
{
  context->state = LLCP_INIT;
  context->dsap = sap;
}

/*
 * Initialize the LLCP conversation state.
 */
void llcp_init_name(llcp_ctx *context, prog_char *service_name)
{
  context->state = LLCP_INIT;
  context->dsap = DSAP_DISC;
  context->service_name = service_name;
}

/*
 * Writes a PDU header without sequence field to buffer.
 */
static void __make_pdu(uint8_t ptype, uint8_t dsap, uint8_t ssap,
                       uint8_t *buffer)
{
  buffer[0] = (dsap << 2) | (ptype >> 2);
  buffer[1] = (ptype << 6) | ssap;
}

/*
 * Extracts the PDU type field from an LLCP packet.
 */
static uint8_t __get_ptype(uint8_t *buffer)
{
  return ((buffer[0] & 0x03) << 2) | (buffer[1] >> 6);
}

/*
 * Returns the number of bytes in the llcp header. Payload starts after this.
 */
uint8_t llcp_header_len(uint8_t *buf)
{
  uint8_t ptype =  __get_ptype(buf);
  return (ptype == PDU_I || ptype == PDU_RR) ? 3 : 2;
}

/*
 * Writes a PDU header (2 bytes) to a service access point.
 */
static void __make_service_pdu(uint8_t ptype, uint8_t dsap, uint8_t *buffer)
{
  __make_pdu(ptype, dsap, SSAP, buffer);
}

/*
 * Writes a info command header.
 * Returns number of bytes written to buf.
 */
static uint8_t __make_info_pdu(uint8_t *buf, uint8_t dsap)
{
  uint8_t *p = buf;
  __make_service_pdu(PDU_I, dsap, p);
  p += 2;
  *p++ = 0x00; // seq no
  return p - buf;
}

/*
 * Determines next LLCP command to send an NDEF record via SNEP based
 * on a very simple state machine. Handles only a single data packet.
 *
 * Arguments:
 *   cmd - buffer to receive the next command to send via NFC
 *   resp - the last response received via NFC
 *   record - payload, e.g. NDEf record
 *   record_len - length of payload in bytes
 *   context - Keeps the conversation state
 *
 * Returns the size of the command string or 0 if no command.
 *
 * A typical conversation with a LLCP(SNEP) handset is as follows:
 * -> SYMM
 * <- CONN [LLCP_CONNECTING]
 * -> SYMM
 * <- SYMM
 * -> CC [LLCP_CONNECTED]
 * <- I SNEP PUT
 * -> RR 1
 * <- SYMM
 * -> I SNEP RESP(0x81) [LLCP_CONFIRMED]
 * <- RR
 * -> SYMM
 * <- DISC [LLCP_DISCONNETING]
 * -> DM 0
 *
 * A typical conversation with a LLCP(NPP) handset is as follows:
 * -> SYMM
 * <- CONN [LLCP_CONNECTING]
 * -> CC [LLCP_CONNECTED]
 * <- I NPP [LLCP_CONFIRMED]
 * <- DISC [LLCP_DISCONNETING]
 * -> DM 0
 */

uint8_t get_llcp_command( uint8_t *cmd, uint8_t *resp, llcp_ctx *context)
{
  uint8_t ptype = __get_ptype(resp);
  switch (context->state) {
    case LLCP_INIT:
      switch (ptype) {
        case PDU_CONNECT:
          // Do not accept connections
          lcd_printf(0, "<- DM");
          __make_service_pdu(PDU_DM, context->dsap, cmd);
          cmd[2] = 0x11; // reason: we do not accept CONN requests
          context->state = LLCP_DONE;
          return 3;

        default:
          // Send CONN to get started (regardless of what we received)
          lcd_printf(0, "<- CONN [0->1] %i", context->dsap);
          context->state = LLCP_CONN_PENDING;
          __make_service_pdu(PDU_CONNECT, context->dsap, cmd);
          if (context->dsap != DSAP_DISC) {
            // Request service by well-known number
            return 2;
          } else {
            // Request service by name, add SN parameter
            cmd[2] = 0x06; // Parameter SN (Service Name)
            strcpy_P((char *)(&cmd[4]), context->service_name);
            cmd[3] = strlen_P(context->service_name);
            return cmd[3] + 4;
          }
      }
      break;

    case LLCP_CONN_PENDING:
      switch (ptype) {
        case PDU_CC:
          // Connection confirmed -> set dsap & reply with I pdu
          // Caller has to append payload data
          context->dsap = resp[1] & 0x1f;
          lcd_printf(0, "-> CC [1] %i", context->dsap);
          uint8_t size = __make_info_pdu(cmd, context->dsap);
          lcd_printf(0, "<- I [1->2]");
          context->state = LLCP_CONNECTED;
          return size;

        case PDU_SYMM:
          // Reply to SYMM with SYMM while waiting for connection.
          lcd_printf(0, "<-> SYMM [1]");
          __make_pdu(PDU_SYMM, 0, 0, cmd);
          return 2;

        case PDU_DM:
          // Connection request not honored
          lcd_printf(0, "-> DM [1] %i", resp[llcp_header_len(resp)]);
          context->state = LLCP_REJECT;
          return 0;

        default:
          lcd_print_hex(1, resp, 8);
      }
      break;

   case LLCP_CONNECTED:
      switch (ptype) {
        case PDU_I:
          // Acknowlegde response from initiator
          lcd_printf(0, "-> I %i [2]", resp[1 + llcp_header_len(resp)]);
          lcd_printf(0, "<- RR [2->3]");
          __make_service_pdu(PDU_RR, DSAP_SNEP, cmd);
          cmd[2] = resp[2]; // Sequence number
          context->state = LLCP_CONFIRMED;
          return 3;

        case PDU_RR:
          // Receive ready from initiator
          lcd_printf(0, "-> RR %i [2]", resp[2]);
          // Reply with SYMM as nothing more to send
          lcd_printf(0, "<- SYMM [2]");
          __make_pdu(PDU_SYMM, 0, 0, cmd);
          return 2;

        case PDU_SYMM:
          // Reply to SYMM with SYMM while waiting for confirmation.
          lcd_printf(0, "<-> SYMM [1]");
          __make_pdu(PDU_SYMM, 0, 0, cmd);
          return 2;

        default:
          lcd_print_hex(1, resp, 8);
      }
      break;

    case LLCP_CONFIRMED:
      // Disconnect.
      lcd_printf(0, "<- DISC [3->4]");
      __make_service_pdu(PDU_DISC, context->dsap, cmd);
      context->state = LLCP_DISCONNECTING;
      return 2;

    case LLCP_DISCONNECTING:
      // We disconnected, so ignore everything except DM.
      if (ptype == PDU_DM) {
        lcd_printf(0, "-> DM %i [4]", resp[2]);
        context->state = LLCP_DONE;
      }
      break;

    case LLCP_DONE:
    case LLCP_REJECT:
      break;
  }
  return 0;
}
