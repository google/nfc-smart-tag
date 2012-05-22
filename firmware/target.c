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
 * Allow the base station to act as a target in the following modes:
 *   Type 3 NFC tag (Felica)
 *   SNEP NDEF Push over LLCP (ISO 18092)
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <util/delay.h>

#include "eeprom_data.h"
#include "nfc/llcp.h"
#include "nfc/npp.h"
#include "nfc/snep.h"
#include "nfc/sp.h"
#include "nfc/type3tag.h"
#include "peripheral/lcd.h"
#include "peripheral/led.h"
#include "peripheral/timer.h"
#include "rcs956/rcs956_protocol.h"
#include "rcs956/rcs956_target.h"
#include "melodies.h"
#include "nfc_url2.h"

#include "target.h"

// Adapter method to pass URL making to Smart Poster
static bool get_url(uint8_t *buf, uint8_t buf_size,
             __attribute__((unused)) void* extra) {
  return build_url((char *)buf, buf_size, NULL);
}

/*
 * Services a LLCP conversation with a BEAM device, such as Android ICS, or
 * an NPP device, such as Android GB. First attempts to connect on well known
 * SNEP service. If that request is denied, connects to the NPP service by
 * name. Once connected, sends the NDEF record via the available protocol.
 *
 * LLCP is a connection oriented protocol. The state is managed by llcp.c.
 * The basic flow is as follows:
 * 1) Connect on SNEP service (4)
 * 2) If fails, connect on NPP service by name
 * 3) Once connected, send payload via SNEP or NPP
 * 4) If SNEP, wait for acknowledgment (NPP does not ackonwledge)
 * 5) Disconnect
 *
 * Arguments:
 *   resp - Buffer to be reused to receive LLCP commands
 *   resp_len - Size of resp in bytes
 *   ndef - Data to send to peer, e.g. a NDEF record
 *   ndef_len - Length of payload in bytes
 *
 * Returns:
 *   true if all data was passed to peer, false on error or timeout
 */
bool llcp_service(uint8_t *resp, int resp_len, uint8_t ndef[], int ndef_len)
{
  uint8_t cmd[160];
  uint8_t cmd_len;
  uint8_t *llcp_resp;
  uint8_t loop_count = MAX_TARGET_LOOP_TIMES;
  uint8_t status;
  bool success = false;
  bool snep = true;
  llcp_ctx context;

  llcp_init_wellknown(&context, DSAP_SNEP);

  do {
    // Get LLCP request (which is the response from RC-S956 command)
    if (!rcs956_tg_get_dep_data(resp, resp_len)) {
      return false;
    }

    // Quit if Status not OK
    if (resp[OFS_DATA] != 0 || resp[OFS_DATA_LEN] < 5) {
      lcd_printf(1, "Status %x", resp[OFS_DATA]);
      return false;
    }

    // Determine response to LLCP command (skip RC-S956 status byte)
    llcp_resp = resp + OFS_DATA + 1;
    cmd_len = get_llcp_command(cmd, llcp_resp, &context);

    // try SNEP first, try NPP next, and give up.
    if (snep) {
      if (context.state == LLCP_CONNECTED) {
        // Add the SNEP payload once we are connected
        cmd_len += snep_put(&cmd[cmd_len], ndef, ndef_len);
      } else if (context.state == LLCP_CONFIRMED) {
        // Check SNEP response status
        if (snep_response_status(llcp_resp+llcp_header_len(llcp_resp)) ==
            SNEP_RESP_SUCCESS) {
          success = true;
        }
      } else if (context.state == LLCP_REJECT) {
        // If peer cannot speak SNEP, start over with NPP
        snep = false;
        llcp_init_name(&context, get_npp_service_name());
        cmd_len = get_llcp_command(cmd, llcp_resp, &context);
      }
    } else { // npp
      if (context.state == LLCP_CONNECTED) {
        // Add the NPP payload once we are connected
        cmd_len += npp(&cmd[cmd_len], ndef, ndef_len);
        // NPP does not wait for confirmation, just declare success
        success = true;
        context.state = LLCP_CONFIRMED;
      } else if (context.state == LLCP_REJECT) {
        return false; // all attempts failed.
      }
    }
    // Send command to peer
    if (cmd_len > 0) {
      rcs956_tg_set_dep_data(cmd, cmd_len, &status);
    }
  } while (context.state != LLCP_DONE && --loop_count);
  return success;
}

/**
 * Emulates an NFC Type 3 tag over NFC-F (Felica) Protocol.
 *
 * Arguments:
 *   resp: shared response buffer. First command comes in this and is reused.
 *   resp_len: length of the buffer (for safe reuse).
 *   ndef: NDEF record, e.g. smart poster data.
 *   ndef_len: length of the NDEf record
 *   card_idm: Used for polling command.
 *
 * Returns:
 *   true if all card data was read by initiator, false on error or timeout
 */
bool felica_service(uint8_t *resp, int resp_len,
                    uint8_t ndef[], int ndef_len,
                    uint8_t card_idm[])
{
  uint8_t cmd[TYPE3_BUFFER_SIZE];
  uint8_t cmd_len;
  uint8_t loop_count = MAX_TARGET_LOOP_TIMES;
  bool has_read_all = false;

  do {
    // The response from the NFC module is the command from the initiator.
    // Skip the status and length byte in the RC-S956 response.
    cmd_len = get_type3_response(
        cmd, &resp[OFS_DATA+2], card_idm, ndef, ndef_len, &has_read_all);

    // Send response if we have one & get next command
    if (cmd_len > 0) {
      if (!rcs956_comm_thru_ex(cmd, cmd_len, resp, resp_len, TG_COMM_TIMEOUT_MS)) {
        return false;
      }
      if (resp[7] == 0x31) {
        lcd_printf(0, "closed");
        return false;
      }
    }
  } while (!has_read_all && --loop_count);

  return has_read_all;
}

/**
 * Switch RC-S620/S into target mode and respond to Felica requests
 * as Type 3 Tag and to ISO 18092 requests with LLCP/SNEP.
 *
 * Can leave LED on to avoid flickering. Main program should turn led
 * off as appropriate.
 *
 * Argument:
 *      label: Label of the NFC type 3 tag (Text record).
 *
 * Returns:
 *      TGT_COMPLETE initiator detected and all data read
 *      TGT_TIMEOUT no initiator detected
 *      TGT_RETRY a tag was found, but info not read or unknown type
 *      TGT_ERROR communication error with RC-S620
 */
enum target_res target(char *label)
{
  uint8_t resp[128];
  uint8_t card_idm[8];
  uint8_t i;

  // Set IDM to (simple) random numbers
  for (i = 0; i < sizeof(card_idm); i++) {
    card_idm[i] = (uint8_t)rand();
  }

  // (1) rcs956_get_firm_version() is called inside rcs956_init.

  // (2)
  if (!rcs956_write_register(0x630d, 0x08)) {
    return TGT_ERROR;
  }

  // (3) Disable ATR_RES from being returned automatically
  if (!rcs956_set_param(0x18)) {
    return TGT_ERROR;
  }

  // (4) Put Pasori into target mode with specified ID's
  if (rcs956_tg_init(card_idm) != 1) {
    return TGT_ERROR;
  }

  if (rcs956_tg_wait_initiator(resp, sizeof(resp)) != 1) {
    return TGT_TIMEOUT;
  }

  // We see an initiator of any kind: give user feedback
  led_on();
  play_melody(melody_click, sizeof(melody_click) / sizeof(struct note));
  lcd_printf(1, "actv mode %02x", resp[OFS_DATA]);
  uint8_t target_type = resp[OFS_DATA] & 0x03; // Target type

  // (5) Turn off target optimization for 106kbps
  if ((resp[OFS_DATA] & 0x70) == 0) {
    if (!rcs956_write_register(0x630d, 0x00)) {
      return TGT_ERROR;
    }
    if (!rcs956_write_register(0x6301, 0x3b)) {
      return TGT_ERROR;
    }
  }

  // (6) Respond to ATR_REQ (d4 00 NFCID) per ECMA-340 (NFCIP-1)
  if (resp[OFS_DATA+1] >= 14 &&  // data length
      resp[OFS_DATA+2] == 0xd4 && // ATR_REQ CMD0
      resp[OFS_DATA+3] == 0x00) { // ATR_REQ CMD1
    // Respond with general bytes to indicate LLCP support
    if (resp[OFS_DATA+1] > 17 && is_llcp_atr_req(resp+OFS_DATA+18)) {
      uint8_t gen_bytes[48];  // max size for tg field
      uint8_t len;
      len = llcp_atr_res_general_bytes(gen_bytes);
      if (!rcs956_tg_set_general_bytes(gen_bytes, len)) {
        return false;
      }
      lcd_puts(1, "llcp");
    } else {
      if (!rcs956_tg_set_general_bytes(NULL, 0)) {
        return false;
      }
    }
  }

  // Respond to a RLS_REQ (d4 0a DID) per ECMA-340 (NFCIP-1)
  if (resp[OFS_DATA+1] >= 3 &&  // data len
      resp[OFS_DATA+2] == 0xd4 &&  // RLS_REQ CMD0
      resp[OFS_DATA+3] == 0x0a) {  // RLS_REQ CMD 1
      uint8_t cmd[48];

      cmd[0] = 3; // size
      cmd[1] = 0xd5; // RLS_RES CMD 0
      cmd[2] = 0x0b; // RLS_RES CMD 1
      cmd[3] = resp[OFS_DATA+4]; // DID
      if (!rcs956_comm_thru_ex(cmd, 4, resp, sizeof(resp), false)) {
        return TGT_ERROR;
      }
      lcd_puts(1, "RLS_REQ");
      return TGT_RETRY;
  }

  // (7)
  if (target_type == 1 || target_type == 2) {
    uint8_t sp[128]; /* maximum tag size is hopefully less than 80 bytes */
    uint8_t sp_len;
    bool success = false;
    sp_len = smart_poster(sp, sizeof(sp), label, get_url, NULL);
    lcd_printf(1, "sp len %i", sp_len);
    start_timer(TIMER_RES_1ms);
    if (target_type == 1) { // LLCP ISO18092
      success = llcp_service(resp, sizeof(resp), sp, sp_len);
    } else if (target_type == 2) { // Felica
      success = felica_service(resp, sizeof(resp), sp, sp_len, card_idm);
    }
    stop_timer();
    if (success) {
      lcd_printf(1, "type %i OK %i ms", target_type, get_timer());
      return TGT_COMPLETE;
    } else {
      lcd_printf(1, "type %i retry", target_type);
      return TGT_RETRY;
    }
  } else {
    lcd_printf(1, "type %d retry", target_type);
    // give the initiator a chance to try another mode
    return TGT_RETRY;
  }
}
