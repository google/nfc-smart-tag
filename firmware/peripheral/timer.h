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
 * Uses 16 bit Timer/Counter 1.
 */

#ifndef TIMER_H_
#define TIMER_H_

// Set the timer slower to compensate for additional run-time due to
// counter interrupt (est. 30 clock cycles).
enum TIMER_RESOLUTION {
    TIMER_RES_CLOCK = 1,
    TIMER_RES_1ms = F_CPU / 1000 + 30,
    TIMER_RES_100us = F_CPU / 10000 + 30
};

// Starts 16 bit timer with the specified resolution.
void start_timer(enum TIMER_RESOLUTION resolution);

// Returns the number of time units passed since start_timer.
unsigned int get_timer();

// Stops the timer. The last timer value is preserved.
void stop_timer();

#endif  // TIMER_H_
