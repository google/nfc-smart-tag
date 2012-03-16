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
 * Drives a HD44780 compatible LCD connected as follows:
 * VCC (JP1-1) --> VCC
 * GND (JP1-2) --> GND, R/W
 * PC0 (JP1-3) --> DB4
 * PC1 (JP1-4) --> DB5
 * PC2 (JP1-5) --> DB6
 * PC3 (JP1-6) --> DB7
 * PC4 (JP1-7) --> RS
 * PC5 (JP1-8) --> EN
 * Pot 0V - 5V --> Contrast (check data sheet)
 *
 * Example data sheet:
 * http://lcd-linux.sourceforge.net/pdfdocs/hd44780.pdf
 *
 * Operates in nibble mode (bytes sent as two nibbles).
 * Links in printf library, which significantly increases code size.
 */

#include <stdio.h>

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"

#define BUF(l,p) __buf[(l)*LINE_LEN+(p)]

// The character buffer
static char __buf[LINE_LEN*MAX_LINE+1];

/*
 * Sends one nibble (4 bits) to the LCD.
 */
static void __send_nibble(char data)
{
  LCD_DATA = (LCD_DATA & 0b11110000) | (0b00001111 & data);
  LCD_CONTROL |= EN;   // enable
  _delay_us(10);
  LCD_CONTROL &= ~EN;  // disable
}

/*
 * Sends a command byte to the LCD.
 */
static void __attribute__((noinline)) __send_cmd(char data)
{
  LCD_CONTROL &= ~RS;  // clear RS
  __send_nibble(data >> 4);
  __send_nibble(data);
}

/*
 * Sends a data byte to the LCD.
 */
static void __attribute__((noinline)) __send_char(char data)
{
  LCD_CONTROL |= RS;  // set RS
  __send_nibble(data >> 4);
  __send_nibble(data);
}

/*
 * Initialize the LCD in 4 bit mode. Waits 100ms for power to stabilize.
 */
void lcd_init(void)
{
  uint8_t i;

  LCD_DDR  = 0b00111111;
  LCD_CONTROL = 0;
  LCD_DATA = 0;

  _delay_ms(100);
  LCD_CONTROL &= ~RS;  // clear RS -> Command
  __send_nibble(0b0011);
  _delay_ms(10);

  __send_nibble(0b0011);
  _delay_ms(1);

  __send_nibble(0b0011);
  _delay_ms(1);

  __send_nibble(0b0010);  // set 4bits mode (DL=0).
  _delay_ms(1);

  // Function Set: DL = 4 bit, 2 lines
  __send_cmd(0b00101000);
  _delay_ms(1);

  // Display off
  __send_cmd(0b00001000);
  _delay_ms(1);

  // Clear Display
  __send_cmd(0b00000001);
  _delay_ms(1);

  // Entry mode: Increment cursor, no shift
  __send_cmd(0b00000110);
  _delay_ms(1);

  // Display on, no cursor, no blink
  __send_cmd(0b00001100);
  _delay_ms(1);

  // Set buffer to all blanks
  for (i = 0; i < sizeof(__buf); i++) {
    __buf[i] = ' ';
  }
}

static void __set_cursor_at(uint8_t line, uint8_t pos)
{
  __send_cmd(0b10000000 | (line << 6) | pos);
  _delay_us(40); // time to complete command
}

static void __write_char(char c)
{
  __send_char(c);
  _delay_us(40); // time to complete command
}

/*
 * Copies one line from buffer to LCD.
 */
static void __update_lcd_line(uint8_t line)
{
  uint8_t pos;

  __set_cursor_at(line, 0);
  for(pos = 0; pos < LINE_LEN; pos++) {
    __write_char(BUF(line,pos));
  }
}

/*
 * Print null terminated string to start of line and clear rest of the line.
 */
void lcd_puts(uint8_t line, char* text)
{
  lcd_puts2(line, 0, text);
}

/*
 * Print null terminated string to line and clear rest of the line.
 */
void lcd_puts2(uint8_t line, uint8_t pos, char* text)
{
  uint8_t i = pos;

  while ((i < LINE_LEN) && (*text != '\0')) {
    BUF(line, i++) = *(text++);
  }

  while (i < LINE_LEN) {
    BUF(line, i++) = ' ';
  }
  __update_lcd_line(line);
}

/*
 * sprintf to specified line
 */
void lcd_printf(uint8_t line, char* format, ...)
{
  va_list vl;
  va_start(vl,format);
  int i = vsprintf(__buf + line * LINE_LEN, format, vl);
  va_end(vl);
  while (i < LINE_LEN) {
    BUF(line, i++) = ' ';
  }
  __update_lcd_line(line);
}

/*
 * Print data as hex to LCD.
 */
void lcd_print_hex(uint8_t line, uint8_t* data, uint8_t len)
{
  char buf[len * 2 + 1];
  int i;

  buf[0] = '\0'; // in case len is zero
  for (i = 0; i < len; i++) {
    sprintf(&buf[i * 2], "%02x", data[i]);
  }
  lcd_puts(line, buf);
}
