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
 * Simple background timing for debugging and testing
 * Uses 16 bit Timer/Counter 1
 */

#include <avr/interrupt.h>
#include <avr/io.h>

#include "timer.h"

static int counter = 0;

ISR(TIMER1_COMPA_vect)
{
  counter++;
}

/*
 * Starts 16 bit timer with the specified resolution from 0 in CTC mode.
 */
void start_timer(enum TIMER_RESOLUTION resolution)
{
  TCCR1B = 0; // stop counter
  counter = 0;
  OCR1A = resolution;
  TCCR1B |= _BV(WGM12); // CTC mode
  TIMSK1 |= _BV(OCIE1A); // Match interrupt enable
  TCCR1B |= _BV(CS11); // set prescaler to CLK/8
}

/*
 * Returns the number of time units passed since start_timer
 */
int get_timer()
{
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
