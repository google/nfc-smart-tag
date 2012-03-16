#ifndef EXPERIMENTAL_NFC_AVR_BASE_AVR_SHA1_H_
#define EXPERIMENTAL_NFC_AVR_BASE_AVR_SHA1_H_

#include <stdint.h>

#define SHA1HashSize 20

typedef uint8_t sha1_hash_t[SHA1HashSize];

void sha1(sha1_hash_t hash, uint8_t *buffer, uint16_t buffer_size);

#endif  // EXPERIMENTAL_NFC_AVR_BASE_AVR_SHA1_H_
