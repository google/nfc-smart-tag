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
 * Controls an LED connected to an output pin.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "led.h"

void led_on(void)
{
  DDRB |= _BV(LED_PORT);
  PORTB |= _BV(LED_PORT);
}

void led_off(void)
{
  DDRB |= _BV(LED_PORT);
  PORTB &= 0xFF ^ _BV(LED_PORT);
}

void led_toggle(void)
{
  DDRB |= _BV(LED_PORT);
  PINB = _BV(LED_PORT);
}

/*
 * Blinks an LED repeatedly using busy wait.
 *
 * delay: multiple of 100ms (approx)
 * count: how many times the light changes the state.
 */
void blink(uint8_t delay_100ms, uint8_t count)
{
  uint8_t i, j;

  for (i = 0; i < count; i++) {
    led_toggle();

    /* avoid linking floating point library. */
    for (j = 0; j < delay_100ms; j++)
      _delay_ms(100);
  }
}
