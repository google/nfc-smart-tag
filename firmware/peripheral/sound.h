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
 * Play melodies on speaker connected to OC0A (Port D6) output.
 */

#ifndef __SOUND_H__
#define __SOUND_H__

#include <avr/pgmspace.h>

#include <stdbool.h>

// Convert frequency to counter value. The following ranges are allowed:
// 3.58MHz: 110 - 28000
// 20MHz: 610 - 156250
// The higher the tone, the less accurate the rendering.
#define SOUND_MAP(freq) ((F_CPU / 64 / (freq)) + 1) / 2

// Compute entry for struct note by specifying tone frequency and
// duration in ms. Same constraints as SOUND_MAP. No overflow checking.
#define NOTE(freq, dur) \
    { SOUND_MAP(freq), (uint16_t)((unsigned long)(dur) * freq / 500) }

// Creates struct note entry for a pause of dur ms
#define PAUSE(dur) { 0, (uint16_t)((dur * (F_CPU / 64 / 256) + 500) / 1000) }

// Data structure to hold one note.
// Use NOTE and PAUSE macros to specify values for this struct.
struct note {
  // Counter value that determines time between phase change.
  // Use SOUND_MAP macro to precompute correct value from tone frequency.
  //
  // 0 value generates pause. The pause is clock speed dependent:
  // duration * 4.5ms @ 3.58MHz
  // duration * 0.8ms @ 20MHz
  prog_uint8_t counter_compare;

  // Duration in 1/2 cycles.
  prog_uint16_t duration;
};

// Some notes for convenience
// From http://www.phy.mtu.edu/~suits/notefreqs.html
#define F_C5 (523)
#define F_D5 (587)
#define F_E5 (659)
#define F_F5 (698)
#define F_G5 (784)
#define F_A5 (880)
#define F_B5 (988)

#define F_C6 (1046)
#define F_D6 (1175)
#define F_E6 (1319)
#define F_F6 (1397)
#define F_G6 (1568)
#define F_A6 (1760)
#define F_B6 (1975)

#define F_C7 (2093)
#define F_D7 (2349)
#define F_E7 (2637)
#define F_F7 (2794)
#define F_G7 (3136)
#define F_A7 (3520)

void play_melody(const struct note *song, uint8_t size);
bool is_melody_playing(void);

#endif /* __SOUND_H__ */
