# Secure URL Generation and Processing #

The NFC Smart Tag generates unique, digitally signed and encrypted URL's, which it transmits to a mobile device via NFC. Because these URL's direct the user to on-line services, such as loyalty or coupon redemption, they should provide protection against the following attacks:

  * Replay attack (opening the same URL twice)
  * Manual manipulation of URL parameters
  * Reverse engineering the data format

The URL contains a set of core parameters:
  * The 64 bit station ID
  * A 32 bit counter value
  * (Optional) The Mobile device's 64 bit NFC ID, e.g. IDm

In addition, transmission of extra data should be allowed, for example:
  * Battery status of the station
  * Error counts, such as hardware failures
  * (Not implemented) Environmental data obtained from sensors

The server side validates the URL, decodes any data, extracts the extra data, and redirects to an application service. If the URL is processed by a mobile application, the application can perform these steps locally, assuming it has access to the key store. A demo server to decode URL's with the key 00..00 is running at http://nfc-smart-tag.appspot.com.

# Implementation #

The following diagram explains how the URL is constructed by the base station and parsed on the server side.

![http://nfc-smart-tag.googlecode.com/git/docs/NfcUrl.png](http://nfc-smart-tag.googlecode.com/git/docs/NfcUrl.png)

  1. Each station is setup with a unique 64 bit station ID and a 128 bit station key, which can be shared, but should also be unique to the station. Each station is also configured to connect to a base URL, typically a dispatcher service.
  1. Each NFC chip inside a mobile device carries a unique 64 bit ID, known as IDm. IDm number ranges are assigned to each manufacturer. See [Felica Technology Code Descriptions](http://www.sony.net/Products/felica/business/tech-support/index.html)
  1. When a phone touches the smart tag, it automatically transmits the phone's IDm.
  1. The smart tags combines the following parameters into byte buffer  and encodes it using the station key (see below for the encoding scheme):
    * Counter value (increased with every touch)
    * Station ID
    * The phone's IDm. This is optional as the IDm may be considered personal information.
    * Extra data, as described above, such as the battery voltage. The extra data is encoded in [Protocol Buffer](http://code.google.com/p/protobuf/) format.
  1. The smart tag base64 encodes the encrypted buffer and appends it to a URL, which it then transmits to the phone via NFC (Felica Push, SNEP, NPP, Tag emulation)
  1. The phone opens the URL and connects to the server. Alternatively, an application could capture the URL and process it.
  1. The server looks up the station key based on the station ID, which is not encrypted and decodes the smart tag data.
  1. The server can extract the "extra data", which is typically related to a station, but not related to an individual request.
  1. If desired, the server can detect duplicate URL's. Because each station has a unique ID and increments the counter value after each successful URL transmission, each request URL is unique. Using the station ID and the counter as unique key allows easy detection of URL reuse, either legitimately (reloading the page) or illegitimately (sharing a URL with other users).
  1. A dispatcher service looks up the intended action associated with this smart tag.
  1. Lastly, the server redirects to the proper web page. This can be done server-side or vie a client redirect, in which case the URL should be signed to prevent users from storing and reusing those URL's.

# Encryption #

The following diagram shows in detail how the encoded URL is constructed by the smart tag (implemented in [enc.c](http://code.google.com/p/nfc-smart-tag/source/browse/firmware/enc.c)):

![http://nfc-smart-tag.googlecode.com/git/docs/NfcUrl2Crypto.png](http://nfc-smart-tag.googlecode.com/git/docs/NfcUrl2Crypto.png)

  1. The input data consists of:
    * The 64 bit station ID
    * A unique 32 bit counter value
    * The mobile device's 64 bit Felica ID (this can be blanked out)
    * Arbitrary data, usually station health data of up to 28 bytes.
  1. The tag computes a 64 bit hash to assure the integrity of the payload. It uses a SHA-1 (implemented in [avr\_sha1.c](http://code.google.com/p/nfc-smart-tag/source/browse/firmware/crypto/avr_sha1.c)), which produces a 160 bit has. Only the first 64 bits are used.
  1. The 64 bit hash is appended to the Felica ID and Arbitrary data, resulting in a data block from 16 to 44 bytes.
  1. The data is encoded block by block using AES-128 (implemented in [avr\_aes\_enc.c](http://code.google.com/p/nfc-smart-tag/source/browse/firmware/crypto/avr_aes_enc.c)) in CTR mode, scrambling each block by the result of an AES-128 encoding of the Station ID, the tag's counter value, and a separate block counter.
  1. Because the Station ID and counter value are needed to decoding, they have to be transmitted separately. The smart tag therefore pre-pends the counter value to the encoded payload data and encodes the first 128 bytes with AES. The station ID remains in clear text as it is required for station key lookup.
  1. Lastly, the station ID is pre-pended, a one byte version number (2) is appended, and the complete data block is encoded in web safe base 64 encoding. This parameter is appended to the base URL and transmitted to the server.

# Computation Cost #
We implemented the encryption based on the original specifications and then optimized. We tried to keep the original algorithm recognizable while ensuring reasonable performance. We only replaced code with assembler if it brings a significant speed-up, e.g. due to use of carry bits or circular shifts.

The computation cost is:
  * AES-128 key setup: ~4000 cycles
  * AES-128 encoding (one block): ~6700 cycles
  * SHA-1: ??? cycles

At 3.69 MHz, the smart tag's default clock, the whole URL construction and encryption takes roughly 20ms for a typical payload of about 40 bytes, which is more than sufficient for NFC communication. Faster AES-128 and SHA-1 implementations for AVR are available (down to ~2500 cycles per block), but were not available under Apache license, so we decided to write our own.