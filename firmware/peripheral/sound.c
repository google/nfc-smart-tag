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
 * Interrupt-driven sound routines. Uses 8-bit counter 0 in CTC mode.
 * Connect piezo buzzer to Pin OC0A (Port D6).
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "sound.h"

/*
 * Global variables for interrupt-driven melody generation
 */

static volatile const struct note *melody; // Frequencies, 0 = pause
static volatile uint8_t melody_size = 0; // Up to 255 sounds in one map
static volatile uint8_t melody_index; // which tone from the melody we are on
static volatile uint16_t sound_count; // how many cycles left for current sound

/*
 * Interrupt handler. Invoked at twice the frequency of the current
 * sound. Advances to the next sound after sound_count iterations.
 * Disables sound when it reaches the end of the sound map.
 */
ISR(TIMER0_COMPA_vect)
{
  if (sound_count == 0) {
    if (melody_index < melody_size) {
      // set number of cycles
      sound_count = pgm_read_word(&(melody[melody_index].duration));
      // reload counter with current frequency
      uint8_t freq = pgm_read_byte(&(melody[melody_index].counter_compare));
      OCR0A = freq;
      TCCR0A = (freq == 0) ?
        0 // If pause, run until FF, do not toggle OC0A
        : _BV(WGM01) | _BV(COM0A0); // CTC mode, toggle OC0A
      melody_index++;
    } else {
      // turn off counter and port (high impedance)
      TCCR0B &= ~(_BV(CS02) | _BV(CS01) | _BV(CS00));
      PORTD &= ~_BV(PORTD6);
      DDRD &= ~_BV(PORTD6);
    }
  } else {
    --sound_count;
  }
}

/*
 * Play a list of tones in the background.
 * Attribute keeps compiler from falsely optimizing away calls to this method.
 *
 * song - see struct note for field description
 * size - number of entries in melody
 */
void play_melody(const struct note *song, uint8_t size)
{
  TCCR0B = 0; // Stop timer while we setup
  melody = song;
  melody_size = size;
  melody_index = 0;
  sound_count =  0;

  OCR0A = 1; // Trigger interupt on next cycle
  TCCR0A = _BV(WGM01); // CTC mode
  DDRD |= _BV(PORTD6); // Set OC0A to output
  TIMSK0 |= _BV(OCIE0A); // Enable CTC interrupt
  sei(); //  Enable global interrupts
  // Start timer at F_CPU/64:
  // 312.5kHz @ 20 MHz
  // 56kHz @ 3.58MHz
  TCCR0B |= _BV(CS01) | _BV(CS00);
}

/*
 * Return true iff a melody is still playing.
 *
 * Returns false just before last interrupt is triggered.
 */
bool is_melody_playing(void)
{
  return melody_index < melody_size || sound_count > 0;
}
