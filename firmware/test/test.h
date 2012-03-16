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

#ifndef __TEST_H__
#define __TEST_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// If condition is false, prints error message and halts
void assert_msg(bool condition, char* msg);

// Halts if condition is false.
void assert(bool condition);

// Include this at the beginning of every test method.
void test(char *name);

// Init test framework. Call before the first test.
void test_init(void);

// Call this after all tests.
void success();

#endif  // __ TEST_H__
