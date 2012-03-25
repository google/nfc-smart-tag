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
 * by the Felica Plug.
 * 
 * http://www.sony.net/Products/felica/business/tech-support
 */

#include <avr/io.h>
#include <util/delay.h>

#include "three_wire.h"

/*
 * Configure serial IO
 */
void twspi_init(void)
{
  TWSPI_DDR |= _BV(TWSPI_SEL);
  TWSPI_DDR |= _BV(TWSPI_CLK);
  TWSPI_DDR |= _BV(TWSPI_SW);
}

/*
 * Resets all I/O pins to high impedance.
 */
void twspi_disable(void)
{
  TWSPI_DDR &= ~_BV(TWSPI_SEL) & ~_BV(TWSPI_CLK) & ~_BV(TWSPI_SW);
}

void rcs801_suspend(void)
{
  TWSPI_PORT &= ~_BV(TWSPI_SW);
}

void rcs801_resume(void)
{
  TWSPI_PORT |= _BV(TWSPI_SW);
  _delay_us(50);
}

bool rcs801_data_ready(void)
{
  return TWSPI_PIN & _BV(TWSPI_IRQ);
}

/*
 * Set SEL pin to low to indicate data transfer from the host and
 * configures the DATA pin as output.
 */
void twspi_begin_send(void)
{
  TWSPI_PORT &= ~_BV(TWSPI_SEL);
  _delay_us(1);
  TWSPI_DDR |= _BV(TWSPI_DATA);
}

/*
 * Configures the DATA pin as input and sets the SEL pin to high to
 * indicate data transfer to the host.
 */
void twspi_end_send(void)
{
  _delay_us(1);
  TWSPI_DDR &= ~_BV(TWSPI_DATA);
  _delay_us(1);
  TWSPI_PORT |= _BV(TWSPI_SEL);
}

/*
 * Sends a single byte to the bus, MSB first. 
 * Max specified bus speed is 1 MHz.
 */
void twspi_send(uint8_t c)
{
  uint8_t i = 8;
  do {
    TWSPI_PORT &= ~_BV(TWSPI_CLK);
    if (c & 0x80) {
      TWSPI_PORT |= _BV(TWSPI_DATA);      
    } else {
      TWSPI_PORT &= ~_BV(TWSPI_DATA);      
    }
    c <<= 1;
    _delay_us(1);
    TWSPI_PORT |= _BV(TWSPI_CLK);
    _delay_us(1);
  } while (--i);
}

/*
 * Sends a memory buffer to the bus.
 */
void twspi_send_buf(const uint8_t* buf, uint8_t len)
{
  do {
    twspi_send(*buf++);
  } while (--len);
}

void twspi_send_buf_p(const prog_char* buf, uint8_t len)
{
  do {
    pgm_read_byte(buf++);
  } while (--len);
}

/*
 * Receives a byte from the bus. Master controls the clock.
 */
uint8_t twspi_get(void)
{
  uint8_t data = 0;

  for (uint8_t i = 0; i < 8; i++) {
    TWSPI_PORT &= ~_BV(TWSPI_CLK);
    _delay_us(1);
    data <<= 1;
    if (TWSPI_PIN & _BV(TWSPI_DATA)) {
      data |= 1;
    }
    TWSPI_PORT |= _BV(TWSPI_CLK);
    _delay_us(1);
  }
  return data;
}

/*
 * Receives a series of bytes from the bus. No time-out condition.
 */
void twspi_get_buf(uint8_t* buf, uint8_t len)
{
  do {
    *buf++ = twspi_get();
  } while (--len);
}