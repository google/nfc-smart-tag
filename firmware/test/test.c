/*
 * Copyright 2011 Google Inc. All Rights Reserved.
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
 * Very rudimentary unit testing running on the target chip.
 * Results are displayed on the LCD connected to JP1.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <avr/io.h>

#include "../peripheral/lcd.h"
#include "../peripheral/sound.h"

int num_tests = 0;

// used to detect memory overwrite
#define MARKER 0xaa

static const struct note PROGMEM melody_success[] = {
  NOTE(F_E6, 80),
  PAUSE(40),
  NOTE(F_G6, 80),
  PAUSE(40),
  NOTE(F_E7, 80),
  PAUSE(40),
  NOTE(F_C7, 80),
  PAUSE(40),
  NOTE(F_D7, 80),
  PAUSE(40),
  NOTE(F_G7, 80)
};

static const struct note PROGMEM melody_fail[] = {
  NOTE(F_E5, 300),
};

/*
 * If condition is false, prints error message and halts
 */
void assert_msg(bool condition, char* msg)
{
  if (!condition) {
    // LED red
    if (msg != NULL) {
      lcd_puts(1, msg);
    } else {
      lcd_puts(1, "FAIL");
    }
    play_melody(melody_fail, sizeof(melody_fail) / sizeof(struct note));
    for (;;);
  }
}

/*
 * Halts if condition is false.
 */
void assert(bool condition)
{
  assert_msg(condition, NULL);
}

/*
 * Add this at the beginning of every test method
 */
void test(char *name)
{
  num_tests++;
  lcd_puts(0, (strlen(name) > 5) ? name + 5 : name);
  lcd_puts(1, "Testing");
}

/*
 * Init test framework. Call before the first test.
 */
void test_init(void)
{
  lcd_init();
}

/*
 * Call this after all tests.
 */
void success() {
  // LED Green
  lcd_printf(1, "%i tests OK!", num_tests);
  play_melody(melody_success, sizeof(melody_success) / sizeof(struct note));
  for (;;);
}
