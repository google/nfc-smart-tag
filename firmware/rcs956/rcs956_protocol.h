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
 * Support for SONY RC-S956 Packet Format and Protocol
 *
 * See http://www.sony.co.jp/Products/felica/business/tech-support
 *
 */

#ifndef __RC956_PROTOCOL_H__
#define __RC956_PROTOCOL_H__

#include <stdint.h>
#include <stdbool.h>

#include <avr/pgmspace.h>

extern enum PROTOCOL_ERROR {
  SUCCESS = 0,
  TIMEOUT,
  BUFFER_EXCEEDED,
  UNEXPECTED_REPLY,
} protocol_errno;


#define MAX_RECV_SIZE (32 + 7)
#define MAX_SEND_SIZE (192 + 7)

#define L8(x) (x & 0xff)
#define H8(x) ((x >> 8) & 0xff)

/*
 * USART_READ_TIMEOUT <= 65535(MAX_COUNTER) * 1024(PRESCALE) * 1000(MS) / F_CPU
 *
 * e.g. 20   MHz:  3355
 *       3.58MHz: 18745
 */
#define USART_READ_TIMEOUT 3000 /* ms */

// The offset of the byte that describes the total packet length
#define OFS_DATA_LEN 3

// The offset of the command code in a  packet to/from RC-S620
#define OFS_CMD 5

// The offset of the sub-command code in a packet to/from RC-S620
#define OFS_SUB_CMD 6

// The offset of the data section in a packet to/from RC-S620
#define OFS_DATA 7

// Send command to RC-S956
bool rcs956_send_command(const uint8_t *cmd, size_t cmd_len);

// Send command from program memory to RC-S956
bool rcs956_send_command_p(const prog_char *cmd, size_t cmd_len);

// Read response from RC-S956 or timeout
bool rcs956_read_response(uint8_t *resp_buffer, size_t resp_buffer_size);

// Cancel a pending command via Ack
void rcs956_cancel_cmd(void);

#endif /* !__RC956_PROTOCOL_H__ */
