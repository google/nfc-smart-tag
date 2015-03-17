# Functional Overview #

The smart tag firmware consists of the following functional blocks:
  * NfcProtocols implementations, such as tag emulation, LLCP, SNEP, etc.
  * Three main operating modes: Peer-to-peer, initiator, target
  * Low level I/O routines, such as serial communication, battery voltage check, power management, sound, etc.
  * Drivers for NFC modules, such as the Sony RC-S956 NFC Chip or Sony RC-S926 "Felica Plug".
  * Encryption routines to digitally sign and protect data (see SecureUrl).
  * Main control for [[SONY NFC Module](http://www.sony.co.jp/Products/felica/business/products/RC-S620.html) active NFC module] (using RC-S956) or [passive Felica Plug](http://www.sony.net/Products/felica/business/products) (using RC-S926)

![http://nfc-smart-tag.googlecode.com/git/docs/FirmwareModules.png](http://nfc-smart-tag.googlecode.com/git/docs/FirmwareModules.png)

Drivers for additional NFC hardware or support for additional NFC protocols can easily be added.

# Code Structure #

  * `nfc` - NFC protocols, SNEP, LLCP, Smart Poster, Felica Push. No external dependencies.
  * `crypto` - AES, SHA1, base62 encoding. No external dependencies.
  * `rcs926` - Driver routines for [SONY Felica Plug](http://www.sony.net/Products/felica/business/products) with RC-S926 chip. No external dependencies.
  * `rcs956` - Driver routines for [SONY NFC Module](http://www.sony.co.jp/Products/felica/business/products/RC-S620.html) with RC-S956 chip. No external dependencies.
  * `peripheral` - General IO, power control, serial, etc. No project dependencies.
  * `test` - On device unit tests (deploy to AVR to run)

# General Comments #


# NFC Protocols #

See NfcProtocols

# RC-S956 #


# Main Loop #