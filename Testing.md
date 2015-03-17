# On Target testing #
Because some routines use assembler code (e.g. crypto) and others depend on hardware features (e.g. EEPROM), we implemented simple testing routines to be run on the target hardware. We also don't trust the compiler across platforms 100%, so we feel good seeing the code run on the actual device.

There isn't much of a framework here, just a number of test routines which at the end report success or failure. It's not meant for real TDD, but mainly as regression tests, especially during optimizations or rewrite of assembler routines.

The On Target tests are contained in the [/test](http://code.google.com/p/nfc-smart-tag/source/browse/#git%2Ffirmware%2Ftest) directory. Building  `make test` creates the `test.hex` image, which contains all tests and outputs results to both the LCD and speaker (melody = success, long beep = failure).


# On Host Testing #
For general logic we run test cases on the host to reduce cycle time.