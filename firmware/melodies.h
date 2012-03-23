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
 * Melodies
 */

#ifndef MELODIES_H_
#define MELODIES_H_

#include "peripheral/sound.h"

static const struct note PROGMEM melody_start_up_battery[] = {
  NOTE(2000, 100),
  NOTE(1000, 200),
};

static const struct note PROGMEM melody_start_up_external[] = {
  NOTE(1000, 150),
  NOTE(2000, 150),
};

static const struct note PROGMEM melody_short_beeps[] = {
  NOTE(2000, 100),
  PAUSE(100),
  NOTE(2000, 100),
  PAUSE(100),
  NOTE(2000, 100),
  PAUSE(100),
  NOTE(2000, 100),
};

static const struct note PROGMEM melody_kayac_beep[] = {
  NOTE(F_C6, 200),
  NOTE(F_E6, 100),
  PAUSE(1200),
};

static const struct note PROGMEM melody_click[] = {
  NOTE(F_F6, 24),
};

static const struct note PROGMEM melody_googlenfc001[] = {
  NOTE(F_G6, 125),
  PAUSE(125),
  NOTE(F_G5, 125),
  PAUSE(125),
  NOTE(F_G6, 125),
  NOTE(F_A6, 125),
  NOTE(F_G6, 125),
  NOTE(F_A6, 125),
  PAUSE(125),
  NOTE(F_G6, 125),
  PAUSE(125),
  NOTE(F_F6, 125),
  NOTE(F_E6, 125),
  PAUSE(125),
  NOTE(F_C6, 150),
  PAUSE(600),
};

/*
 * Play a series of short beeps (up to 4).
 * Does not wait for melody to complete.
 */
inline static void beep_n_times(uint8_t count) {
  // Beep and pause are two entries, so multiply by 2. Skip last pause.
  play_melody(melody_short_beeps, (count << 1) - 1);
}

#endif  // MELODIES_H_
