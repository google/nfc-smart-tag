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
 * Emulate a NFC Smart Tag using a Felica Plug with RC-S926 Chip
 *
 * http://www.sony.net/Products/felica/business/tech-support
 */

#include <string.h>
#include <stdbool.h>

#include <avr/sleep.h>
#include <util/delay.h>

#include "nfc_url2.h"
#include "melodies.h"
#include "nfc/sp.h"
#include "peripheral/lcd.h"
#include "peripheral/power_down.h"
#include "peripheral/sound.h"
#include "peripheral/three_wire.h"
#include "rcs801/rcs801.h"

#define PLUG_URL "http://www.google.com?q=nfc"

#define INACTIVE_CTR 500

static bool make_url(uint8_t *buf,
                     __attribute__((unused)) uint8_t buf_size,
                     __attribute__((unused)) void* extra) {
  memcpy((char *)buf, PLUG_URL, strlen(PLUG_URL));
  return strlen(PLUG_URL);
}

static void sleep_until_melody_completes(void)
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  // In this tight loop we are guaranteed one last interrupt
  // after is_melody_playing() returns false
  while (is_melody_playing()) {
    sleep_mode();
  }
}

int main() {
  uint8_t ndef[128];
  uint8_t ndef_len;
  uint16_t inactive_ctr = 0;
  bool active = false;
  bool has_read_all = false;

  twspi_init();
  _delay_ms(100);

  ndef_len = smart_poster(ndef, sizeof(ndef), NULL, &make_url, NULL);

  lcd_init();
  lcd_puts(0, "Felica Plug");
  lcd_printf(0, "URL %iB", ndef_len);

  beep_n_times(2);
  
  // Wake up if RF field detected
  rcs926_wake_up_on_rf();
  disable_unused_circuits();

  for (;;) {
    if (inactive_ctr == 0) {
      rcs926_suspend();
      lcd_puts(0, "sleep");
      active = false;
      // RF detect will wake up
      sleep_forever();
    } else {
      --inactive_ctr;
    }

    if (rcs926_rf_present()) {
      inactive_ctr = INACTIVE_CTR;
      if (!active) {
        rcs926_resume();
        rcs926_init();
        lcd_puts(0, "active");
        active = true;
      }
    }
    if (active) {
      if (rcs926_data_ready()) {
        rcs926_process_command(ndef, ndef_len, &has_read_all);
        if (has_read_all) {
          play_melody(melody_googlenfc001,
                      sizeof(melody_googlenfc001) / sizeof(struct note));
          sleep_until_melody_completes();
          active = false;
          has_read_all = false;
          lcd_puts(0, "success");
        }
      }
    }
    // TODO: replace with sleep until data ready or timer
    _delay_ms(2);
  }

}
