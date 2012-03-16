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
 * Read and write using built-in USART.
 *
 * Uses synchronous send and asynchronous (interrupt-driven) receive.
 * Does not check for receive buffer overflow.
 */

#ifndef __USART__H__
#define __USART__H__

#include <stdbool.h>
#include <stdint.h>

#include <avr/pgmspace.h>

/* Size of receive data buffer (must be power of 2) */
/* At 115200baud we receive at most ~11bytes/ms */
#define RECEIVE_BUFFER_SIZE 32

void usart_init(void);
void usart_disable(void);
void usart_clear_receive_buffer(void);

/* Receive (asynchronous) */
bool usart_has_data(void);
uint8_t usart_get(void);

/* Send (synchronous) */
void usart_send(uint8_t c);
void usart_send_buf(const uint8_t* buf, int len);
void usart_send_buf_p(const prog_char* buf,int len);

#endif /*__USART__H__ */
