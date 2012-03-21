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
 * Constructs a digitally signed one-time URL.
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "enc.h"
#include "eeprom_data.h"
#include "crypto/ws_base64_enc.h"
#include "peripheral/eeprom.h"
#include "proto/base_station.pb.h"

#include "nfc_url2.h"

static uint8_t battery_voltage = 0;

#ifdef WITHOUT_V_FIELD
#define __build_v_param(X, Y, Z, H) true
#else /* !WITHOUT_V_FIELD */

#define MAX_ARBITRARY_SIZE 28

/*
 * Add arbitrary data in protocol buffer format
 */
static void __fill_proto(uint8_t **tmpp, uint8_t *end)
{
  serialize_NfcBaseStationInfo__number_watchdog(tmpp, end,
      eeprom_read_number_wdrf());
  serialize_NfcBaseStationInfo__number_external_reset(tmpp, end,
      eeprom_read_number_extrf());
  serialize_NfcBaseStationInfo__number_power_reset(tmpp, end,
      eeprom_read_number_porf());
  serialize_NfcBaseStationInfo__number_serial_failure(tmpp, end,
      eeprom_read_number_usart_fail());
  serialize_NfcBaseStationInfo__number_brown_out(tmpp, end,
      eeprom_read_number_borf());
  if (battery_voltage > 0) {
    serialize_NfcBaseStationInfo__battery_voltage(tmpp, end,
        battery_voltage);
  }
}

/**
 * Generate URL paramter encoded with NFC URL version 2.
 */
static bool __build_v_param(char *url_buffer, size_t url_buffer_size,
                            uint8_t *idm, uint8_t version)
{
  /*
   * Use station key to AES-CTR encrypt one block with:
   * felica ID (64 bit)
   * arbitrary data (0 - 224 bit)
   * sha1 hash value (64 bit)
   *
   * Use station key to AES-ECB encrypt one block with:
   * counter (32 bit)
   *
   * non encrypt:
   * station id (64 bit)
   * version (8 bit)
   */

  uint8_t data[MAX_ARBITRARY_SIZE + 20 + STATION_ID_BYTES + 1];
  uint8_t station_key[STATION_KEY_BYTES];
  int length = 0;

  /* 64 bit station id */
  eeprom_read_station_info(&data[length], station_key);
  length += STATION_ID_BYTES;

  /* 32 bit counter */
  do {
    uint32_t counter;
    eeprom_increment_counter(&counter);
    memcpy(&data[length], &counter, sizeof(counter));
    length += sizeof(counter);
  } while (0);

  /* 64 bit felica id */
  if (idm != NULL) {
    memcpy(&data[length], idm, IDM_BYTES);
  } else {
    memset(&data[length], 0, IDM_BYTES);
  }
  length += IDM_BYTES;

  /* arbitrary data */
  do {
    uint8_t *tmpp = &data[length];
    uint8_t *end = &data[length + MAX_ARBITRARY_SIZE];
    __fill_proto(&tmpp, end);
    length += (tmpp - &data[length]);
  } while(0);

  /* 64 bit sha1 hash */
  hash64(&data[length], data, length);
  length += HASH_SIZE;

  /* AES128-CTR */
  do {
    uint8_t counter[16];
    /* envelope: station id + counter at the beginning */
    uint8_t envelope_length = (64 + 32) / 8;
    memset(counter, 0, sizeof(counter));
    memcpy(counter, data, envelope_length);
    enc128_ctr(&data[envelope_length], length - envelope_length,
               station_key, counter);
  } while (0);

  /* AES128 counter */
  enc128(&data[STATION_ID_BYTES], station_key);

  /* version */
  memcpy(&data[length], &version, sizeof(version));
  length += sizeof(version);

  return websafe_base64_encode(url_buffer, url_buffer_size, data, length);
}
#endif /* WITHOUT_V_FIELD */

/*
 * Build URL and store in supplied buffer.
 * Returns true on success.
 */
bool build_url(char *url_buffer, size_t url_buffer_size,
               uint8_t __attribute__((unused)) *idm)
{
  /* check for enough buffer space */
  if (url_buffer_size <= sizeof(URL))
    return false;

  memcpy(url_buffer, URL, sizeof(URL));

  return __build_v_param(&url_buffer[sizeof(URL) - 1],
                         url_buffer_size - sizeof(URL),
                         idm, URL_VERSION);
}

/*
 * Set additional data to be transmitted with the URL.
 */
void set_extra_url_data(uint8_t voltage)
{
  battery_voltage = voltage;
}
