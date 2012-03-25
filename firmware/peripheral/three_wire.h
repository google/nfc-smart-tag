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
 * Read and write data using a half duplex three-wire interface used
 * by the Felica Plug. This interface is similar to SPI, but uses only a 
 * single data line.
 * 
 * http://www.sony.net/Products/felica/business/tech-support
 */

#ifndef __THREE_WIRE__H__
#define __THREE_WIRE__H__

#include <stdbool.h>
#include <stdint.h>

#include <avr/pgmspace.h>

#define TWSPI_PORT PORTB
#define TWSPI_DDR DDRB
#define TWSPI_PIN PINB
#define TWSPI_DATA PB0  // Data (in-out)
#define TWSPI_CLK PB1  // Clock (out)
#define TWSPI_SEL PB2  // Read(H)-Write(L) (out)
#define TWSPI_SW PB3  // Stand by (L) (out)
#define TWSPI_RFDET PB4  // RF Signal detected (L) (in)
#define TWSPI_IRQ PB5  // Data ready (H) (in)

// Setup
void twspi_init(void);
void twspi_disable(void);

// Activate and deactivate (suspend mode)
void rcs801_suspend(void);
void rcs801_resume(void);
bool rcs801_data_ready(void);

// Transmit data
void twspi_begin_send(void);
void twspi_end_send(void);

void twspi_send(uint8_t c);
void twspi_send_buf(const uint8_t* buf, uint8_t len);
void twspi_send_buf_p(const prog_char* buf, uint8_t len);

// Receive data
uint8_t twspi_get(void);
void twspi_get_buf(uint8_t* buf, uint8_t len);


#endif /*__THREE_WIRE__H__ */
