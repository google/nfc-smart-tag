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
 * Emulate a NFC Smart Tag using a Felica Plug with RC-S926 Chip,
 * e.g. RC-S801 or RC-S802 Felica Plug
 *
 * http://www.sony.net/Products/felica/business/tech-support
 *
 * Battery consumption in standby mode is ~0.2uA, allowing this to run
 * a long time off a 3V cell battery, e.g. CR2032
 */

#include <string.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "nfc_url2.h"
#include "melodies.h"
#include "nfc/sp.h"
#include "peripheral/lcd.h"
#include "peripheral/power_down.h"
#include "peripheral/sound.h"
#include "peripheral/three_wire.h"
#include "rcs926/rcs926.h"

static bool make_url(uint8_t *buf, uint8_t buf_size,
                     __attribute__((unused)) void* extra) {
  return build_url((char *)buf, buf_size, NULL);
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

/*
 * Main routine to emulate a Type 3 tag.
 */
int main() {
  uint8_t ndef[128];
  uint8_t ndef_len;

  twspi_init();
  _delay_ms(100);

  ndef_len = smart_poster(ndef, sizeof(ndef), NULL, &make_url, NULL);

  lcd_init();
  lcd_puts(0, "Felica Plug");
  lcd_printf(0, "URL %iB", ndef_len);

  beep_n_times(2);
  sleep_until_melody_completes();

  disable_unused_circuits();

  for (;;) {
    lcd_puts(0, "suspend");
    // Wake up if RF field detected
    rcs926_wake_up_on_rf(true);
    rcs926_wake_up_on_irq(false);
    rcs926_suspend();
    // RF detect will wake up
    sleep_forever();

    // If there is not RF, go back to sleep
    if (rcs926_rf_present()) {
      // Wake up Plug
      lcd_puts(0, "resume");
      rcs926_resume();
      rcs926_init();
      // Configure to wake up on data ready (IRQ)
      rcs926_wake_up_on_rf(false);
      rcs926_wake_up_on_irq(true);

      uint8_t loop = 1;
      do {
        // Sleep until timeout (584ms) or data ready (IRQ)
        sleep_until_timer(SLEEP_MODE_PWR_SAVE, true);

        // If we have data, process the command
        if (rcs926_data_ready()) {
          lcd_printf(0, "counter %i", TCNT2);
          loop = 1;
          bool has_read_all = false;
          rcs926_process_command(ndef, ndef_len, &has_read_all);
          if (has_read_all) {
            play_melody(melody_googlenfc001,
                        sizeof(melody_googlenfc001) / sizeof(struct note));
            sleep_until_melody_completes();
            lcd_puts(0, "success");
            // Melody is long enough to complete transfer. Go back to sleep.
            break;
          }
        } else {
          // Time out, give it another chance
          --loop;
          lcd_printf(0, "timeout %i", TCNT2);
        }
      } while (loop);
    }
  }
  return 0;  // unreachable
}
