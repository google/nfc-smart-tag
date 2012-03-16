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
 * Drives a character LCD based on the popular SC1602 chip.
 *
 * Because the printf libraries increase code size significantly, the
 * behavior is guarded by the HAS_LCD compiler switch. This allows the
 * LCD to be optionally enabled for debugging.
 */

#ifndef __LCD_H__
#define __LCD_H__

#ifdef HAS_LCD

#include <avr/io.h>

// Display size
#define LINE_LEN 16
#define MAX_LINE 2

// Port assignments
#define LCD_CONTROL PORTC
#define RS          (_BV(PORTC4))
#define EN          (_BV(PORTC5))
#define LCD_DATA    PORTC /* Use bits 0 -3 */
#define LCD_DDR     DDRC

/* Initializes display. */
void lcd_init(void);

/* Prints null terminated string to specified line */
void lcd_puts(uint8_t line, char* text);

/* Prints null terminated string to specified line and position */
void lcd_puts2(uint8_t line, uint8_t pos, char* text);

/* sprintf to line */
void lcd_printf(uint8_t line, char* format, ...);

/* Print data as hex to LCD */
void lcd_print_hex(uint8_t line, uint8_t* data, uint8_t len);

#else /* !HAS_LCD */
#define lcd_init()
#define lcd_puts(X, Y)
#define lcd_puts2(X, Y, Z)
#define lcd_print_hex(X, Y, Z)
#define lcd_printf(...)
#endif /* HAS_LCD */

#endif /* __LED_H__ */
