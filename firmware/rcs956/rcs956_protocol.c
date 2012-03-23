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
 * Support for SONY RC-S956 Packet Format and Protocol.
 * This chip is used inside the SONY RC-S620 NFC Module.
 *
 * See http://www.sony.co.jp/Products/felica/business/tech-support
 */

#include <string.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "../peripheral/usart.h"

#include "rcs956_protocol.h"

/* Error code to be accessed globally. */
enum PROTOCOL_ERROR protocol_errno;

/* Command codes */
static const prog_char __packet_header[] = { 0x00, 0x00, 0xff };
static const prog_char __packet_footer[] = { 0x00 };
static const prog_char __cmd_ack[] = { 0x00, 0x00, 0xff, 0x00, 0xff, 0x00 };

/*
 * Compute checksum so that sum of bytes plus checksum yields 0.
 */
static uint8_t __checksum_base(const uint8_t *data, size_t len)
{
  uint8_t chksum = 0;
  do {
    chksum -= *data++;
  } while (--len);
  return chksum;
}

/*
 * Read response from RC-S620/S. Waits max of USART_READ_TIMEOUT before
 * timing out.
 *
 * Arguments:
 * resp_buffer: buffer to save response.
 * resp_buffer_size: size of the buffer. (at least 6 bytes)
 *
 * Returns:
 * size of the response if succeeded.
 * Otherwise, 0, protocol_errorno contains error code.
 */
static size_t __read_response(uint8_t *resp_buffer, size_t resp_buffer_size)
{
  size_t i;
  size_t read_size = resp_buffer_size;
  uint16_t timeout_counter = 0;
  /*
   * Read data format (Host Packet Format - Normal Frame)):
   * 0x00    : 0x00             (Preamble)
   * 0x01    : 0x00             (Start of Packet)
   * 0x02    : 0xff
   * 0x03    : Length of data   (LEN)
   * 0x04    : checksum of LEN  (LCS)
   * 0x05    : data             (max 255 bytes)
   * LEN+0x05: checksum of data (DCS)
   * LEN+0x06: 0x00             (Postamble)
   */

  // We need at least enough room to read the packet length
  if (resp_buffer_size <= OFS_DATA_LEN) {
    rcs956_cancel_cmd();
    protocol_errno = BUFFER_EXCEEDED;
    return 0;
  }

  for (i = 0; i < read_size; i++) {
    while (!usart_has_data()) { /* check timeout */
      if (timeout_counter++ > USART_READ_TIMEOUT * 10) {
        rcs956_cancel_cmd();
        protocol_errno = TIMEOUT;
        return 0;
      }
      _delay_us(100); /* 1.44 bytes delay at 115200 bps */
    }
    resp_buffer[i] = usart_get();
    // How many bytes to expect
    if (i == OFS_DATA_LEN) {
      read_size = (resp_buffer[i] == 0) ? 6 : (resp_buffer[i] + 7);
      if (read_size > resp_buffer_size) {
        rcs956_cancel_cmd();
        protocol_errno = BUFFER_EXCEEDED;
        return 0;
      }
    }
  }

  // Todo: verify checksum (note: no checksum if resp_buffer[3] == 0)
  return read_size;
}

/**
 * Send a command to Felica module. Wait max of USART_READ_TIMEOUT for ACK
 * from module before timing out and returning an error.
 *
 * Arguments:
 * cmd: command bytes to send.
 * cmd_len: length of the bytes.
 */
bool rcs956_send_command(const uint8_t *cmd, size_t cmd_len)
{
  size_t resp_size;
  uint8_t resp_buffer[8];
  /*
   * send data format(normal frame):
   * 0x00: 0x00             (Preamble)
   * 0x01: 0x00             (Start of Packet)
   * 0x02: 0xff
   * 0x03: Length of data   (LEN)
   * 0x04: checksum of LEN  (LCS)
   * 0x05: data             (max 255 bytes)
   * 0x06: checksum of data (DCS)
   * 0x07: 0x00             (Postamble)
   */

  // Preamble and Start of Packet
  usart_send_buf_p(__packet_header,(int)sizeof(__packet_header));

  // length of command
  usart_send(cmd_len);

  // checksum of length
  usart_send(0x100 - cmd_len);

  // command
  usart_send_buf(cmd,cmd_len);

  // checksum of command
  usart_send(__checksum_base(cmd,cmd_len));

  // Postamble
  usart_send_buf_p(__packet_footer,sizeof(__packet_footer));

  // ACK: 00 00 ff 00 ff 00
  resp_size = __read_response(resp_buffer, sizeof(resp_buffer));
  if (resp_size == 0) { /* __read_response failed */
    return false;
  } else if (resp_size != 6) {
    protocol_errno = UNEXPECTED_REPLY;
    return false;
  } else {
    return true;
  }
}

/**
 * Send a command to Felica module. Wait max of USART_READ_TIMEOUT for ACK
 * from module before timing out and returning an error.
 *
 * Arguments:
 * cmd: command bytes in the program memory to send.
 * cmd_len: length of the bytes.
 */
bool rcs956_send_command_p(const prog_char *cmd, size_t cmd_len)
{
  uint8_t buf[cmd_len];
  memcpy_P(buf,cmd,cmd_len);
  return rcs956_send_command(buf,cmd_len);
}

/*
 * response data:
 * 0x00: 0x00
 * 0x01: 0x00
 * 0x02: 0xff
 * 0x03: size of response
 * 0x04: 0x100 - size of response
 * 0x05: command (0xd5 succeeded, 0x7f for error)
 * 0x06: subcommand
 * 0x07 -- size + 0x4: payload
 * size + 0x05: checksum
 * size + 0x06: 0x00
 *
 * payload:
 * - polling
 *   0x07 -- 0x09: ???
 *   0x0a: 0x01 if succeeded
 *   0x0b -- 0x12: IDm
 *   0x13 -- 0x1a: PMm
 * - others:
 *   0x07 -- 0x08: ???
 *   0x09: answer command (usually request command number + 1)
 *   0x0a -- 0x11: IDm
 */
bool rcs956_read_response(uint8_t *resp_buffer, size_t resp_buffer_size)
{
  // The real response
  /* 00 00 ff len csum d4 cmd status payload(>=0) csum 00 */
  size_t resp_size = __read_response(resp_buffer, resp_buffer_size);
  if (resp_size == 0) { /* __read_response failed */
    return false;
  } else if (resp_size < 9) {
    protocol_errno = UNEXPECTED_REPLY;
    return false;
  } else {
    return true;
  }
}

/**
 * Send ACK to Felica module, which cancels any pending command. Flushes the
 * receive buffer because the RC-S620 may transmit data while we send the
 * ACK.
 *
 * Sending ACK against receiving a response from Felica module is optional.
 * However, we SHOULD send ACK when we send sleep command.
 */
void rcs956_cancel_cmd(void)
{
  usart_send_buf_p(__cmd_ack,(int)sizeof(__cmd_ack));
  /* 5 ms wait below are:
   * - 1 ms for transfering the ACK command to the RC-S620.
   * - 1 ms for the command execution in the RC-S620. (See Section 3.2.2)
   * - 3 ms for pessimistically waiting for receiving all reply before ACK.
   */
  _delay_ms(5);
  usart_clear_receive_buffer();
}
