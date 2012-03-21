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
 * Simple background timing for debugging and testing. Timers cannot nest.
 * Values are approximate. Interrupt handling (minimally) slows processing,
 * about 20 clock cycles per interrupt.
 *
 * Uses 16 bit Timer/Counter 1
 */

#include <stdbool.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "timer.h"

static unsigned int counter = 0;
static bool raw = false;

ISR(TIMER1_COMPA_vect)
{
  ++counter;
}

/*
 * Starts 16 bit timer with the specified resolution.
 * Uses CTC mode for real time, normal mode for clock count.
 */
void start_timer(enum TIMER_RESOLUTION resolution)
{
  TCCR1B = 0;  // Stop counter
  TCNT1 = 0;
  counter = 0;
  if (resolution == TIMER_RES_CLOCK) {
    TIMSK1 = 0;  // No timer interrupts
    raw = true;
  } else {
    OCR1A = resolution;
    TCCR1B |= _BV(WGM12);  // CTC mode
    TIMSK1 |= _BV(OCIE1A);  // Match interrupt enable
    sei();
  }
  TCCR1B |= _BV(CS10);  // Clock Select = CLK
}

/*
 * Returns the number of time units passed since start_timer
 */
unsigned int get_timer()
{
  if (raw)
    return TCNT1 - 5;  // compensate for function call
  else
    return counter;
}

/*
 * Stops the timer. The last timer value is preserved.
 */
void stop_timer()
{
  TCCR1B = 0;
  // Disable interrupt in case another module uses the timer
  TIMSK1 &= ~_BV(OCIE1A);
}
