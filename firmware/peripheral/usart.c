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

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#include "usart.h"

static volatile uint8_t __usart_buffer[RECEIVE_BUFFER_SIZE];
static volatile uint8_t __usart_buffer_write_index;
static uint8_t __usart_buffer_read_index;

/*
 * Configure serial IO
 */
void usart_init(void)
{
  uint8_t clock_divider;

  /* Set baud rate to 115200 baud */
#if F_CPU == 3580000
  clock_divider = 1;
#elif F_CPU == 12000000
  clock_divider = 6;  // 7.5% error
#elif F_CPU == 16000000
  clock_divider = 8;  // 3.7% error
#elif F_CPU == 20000000
  clock_divider = 10;
#else
  #error "Not supported frequency"
#endif

  UBRR0H = 0;
  UBRR0L = clock_divider;

  /* Enable receiver and transmitter + rx interrupt */
  UCSR0B = _BV(RXEN0) | _BV(RXCIE0) | _BV(TXEN0);

  /* 8 data bits, no parity, 1 stop bit */
  UCSR0C = (3 << UCSZ00);

  sei();
}

/*
 * Turns off the USART and sets the I/O pins to high impedance.
 */
void usart_disable(void)
{
  UCSR0B = 0;
  DDRD &= ~(PORTD0 | PORTD1);
}

/*
 * Returns true iff the read buffer contains at least one byte.
 */
bool usart_has_data(void)
{
  return (__usart_buffer_write_index != __usart_buffer_read_index);
}

/*
 * Reads one byte from the receive buffer.
 * Waits in idle mode if buffer is empty.
 */
uint8_t usart_get(void)
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  while (!usart_has_data()) {
    sleep_mode();
  }

  return __usart_buffer[__usart_buffer_read_index++
      & (RECEIVE_BUFFER_SIZE - 1)];
}

#ifdef __atmega644p__
ISR(USART0_RX_vect)
#else
ISR(USART_RX_vect)
#endif
{
  __usart_buffer[__usart_buffer_write_index++ & (RECEIVE_BUFFER_SIZE - 1)]
      = UDR0;
}

/*
 * Sends one byte via USART. Waits in busy loop if not ready.
 */
void usart_send(uint8_t c)
{
  while (!(UCSR0A & (1 << UDRE0)))
    ;
  UDR0 = c;
}

/*
 * Sends a sequence of bytes via USART.
 */
void usart_send_buf(const uint8_t* buf, int len)
{
  int i;
  for (i = 0; i < len; i++) {
    usart_send(*buf++);
  }
}

/*
 * Sends a sequence of bytes from program memory via USART.
 */
void usart_send_buf_p(const prog_char* buf,int len)
{
  int i;
  for (i = 0; i < len; i++) {
    usart_send(pgm_read_byte(buf++));
  }
}

/*
 * Empties the receive buffer.
 */
void usart_clear_receive_buffer(void)
{
  cli();
  __usart_buffer_write_index = __usart_buffer_read_index = 0;
  sei();
}
