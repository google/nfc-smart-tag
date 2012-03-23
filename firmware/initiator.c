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
 * Initiator feature of the smart tag.
 * Polls for a Felica-equipeed mobile phone and pushes a URL.
 */

#include <stdbool.h>
#include <string.h>

#include "nfc_url2.h"
#include "initiator.h"
#include "nfc/felica_push.h"
#include "peripheral/lcd.h"
#include "peripheral/led.h"
#include "peripheral/timer.h"
#include "peripheral/usart.h"
#include "rcs956/rcs956_initiator.h"
#include "rcs956/rcs956_protocol.h"

/*
 * For only polling Mobile Felica, in other words Osaifu-Keitai, the poll
 * function is called with the mobile syscode (SYSCODE_MOBILE).
 */
#define SYSCODE_MOBILE 0xfe0f

// Adapter method to create URL in place
static uint8_t get_url(uint8_t *buf, uint8_t buf_size, void *extra)
{
  // Pass extra arg as IDm
  if (build_url((char *)buf, buf_size, (uint8_t *)extra)) {
    return strlen((char *)buf);
  } else {
    return 0;
  }
}

/*
 * Main initiator feature. Pools for phone and pushes URL.
 *
 * push_label: Label for a 'keitai' coupon. Only used by a KDDI phone.
 * Returns: false if polling times out or push fails specified number of times,
 *          true if successful URL push.
 */
bool initiator(const char push_label[])
{
  uint8_t idm[IDM_LENGTH];
  uint8_t idm_previous[IDM_LENGTH];
  uint8_t buffer[URL_LENGTH+30]; // extra for label + header
  uint8_t resp[128];
  uint8_t len = 0;
  bool pushed_url = false;
  bool detected_phone;
  uint8_t number_retries = 0;

  memset(idm_previous, 0, IDM_LENGTH);
  do {
    lcd_puts(0, "POLL");
    start_timer(TIMER_RES_1ms);
    detected_phone = initiator_poll(idm, NULL, SYSCODE_MOBILE);
    stop_timer();
    if (!detected_phone) {
      break;
    }
    // Phone detected
    lcd_printf(0, "PUSH URL  %ims", get_timer());
    lcd_print_hex(1, idm, IDM_LENGTH);
    led_on();

    // Do not recompute URL for same phone (keep same counter)
    if (memcmp(idm, idm_previous, IDM_LENGTH) != 0) {
      uint8_t *id = idm;
#ifdef FAKE_IDM
      id = NULL;
#endif /* FAKE_IDM */
      start_timer(TIMER_RES_1ms);
      len = felica_push_url(buffer, sizeof(buffer),
                            idm, get_url, id, push_label);
      memcpy(idm_previous, idm, IDM_LENGTH);
      stop_timer();
      lcd_printf(0, "URL %ims %iB", get_timer(), len);
    }
    initiator_command(buffer, len, resp, sizeof(resp), COMM_TIMEOUT_MS);
    pushed_url = is_felica_push_response(resp + OFS_DATA + 1, len);
    rcs956_rf_off(); // seems to be needed for reseting status in Android.
  } while (!pushed_url && number_retries++ < NUM_RETRY_INITIATOR_LOOP);
  led_off();
  return pushed_url;
}

/**
 * Set default values for time-out to Felica module.
 */
void initiator_set_defaults()
{
  rcs956_rf_off();
  rcs956_set_retry(NUM_RETRY_POLL);
  rcs956_set_retry_com(NUM_RETRY_COMM);
  rcs956_set_timeout(TIMEOUT_STYLE);
}
