#
# Copyright 2012 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

MCU ?= atmega328p
F_CPU ?= 3580000   	# 3.58 MHz

# Name of the Hex image to generate
IMAGE=station

STATION_OBJS = \
       eeprom_data.o \
       enc.o \
       initiator.o \
       nfc/felica_push.o \
       nfc_url2.o \
       proto/base_station.pb.o \
       station_rcs956.o

RCS956_OBJS = \
       peripheral/module_power.o \
       peripheral/usart.o \
       rcs956/rcs956_common.o \
       rcs956/rcs956_initiator.o \
       rcs956/rcs956_protocol.o \
       rcs956/rcs956_target.o

RCS926_OBJS = \
       peripheral/three_wire.o \
       rcs926/rcs926.o

PLUG_OBJS = \
       eeprom_data.o \
       enc.o \
       nfc_url2.o \
       nfc/sp.o \
       nfc/type3tag.o \
       proto/base_station.pb.o \
       station_rcs926.o

LIB_OBJS = \
       crypto/avr_aes_enc.o \
       crypto/avr_sha1.o \
       crypto/ws_base64_enc.o \
       peripheral/battery.o \
       peripheral/eeprom.o \
       peripheral/led.o \
       peripheral/power_down.o \
       peripheral/sound.o \
       peripheral/switch.o \
       peripheral/timer.o

TARGET_OBJS = \
       nfc/llcp.o \
       nfc/npp.o \
       nfc/snep.o \
       nfc/sp.o \
       nfc/type3tag.o \
       target.o

TEST_OBJS= \
       nfc/felica_push.o \
       nfc/llcp.o \
       nfc/type3tag.o \
       peripheral/lcd.o \
       test/all_tests.o \
       test/avr_aes_enc_test.o \
       test/avr_sha1_test.o \
       test/eeprom_test.o \
       test/felica_push_test.o \
       test/llcp_test.o \
       test/test.o \
       test/ws_base64_enc_test.o

# Start empty. Objects are added per command line switches
DEBUG_OBJS =

ALL_OBJS = $(STATION_OBJS) $(PLUG_OBJS) \
           $(LIB_OBJS) $(TARGET_OBJS) \
           $(RCS956_OBJS) $(RCS926_OBS) \
           $(TEST_OBJS)

EEP = $(IMAGE).eep

#================= Main Targets =====================

# Default target.
all: 	begin gccversion \
	station-active \
	station-activepassive \
	station-plug \
	test \
	finished end

# Base Station Binary Active Only (for Felica phones)
station-active: station-active.hex station-active.eep

# Base Station Binary Acite & Passive (for all phones)
station-activepassive: CFLAGS += -DWITH_TARGET
station-activepassive: station-activepassive.hex station-activepassive.eep

# Felica Plug version (for Android NFC phones)
station-plug: station-plug.hex

# On chip tests
test: CFLAGS += -DHAS_LCD
test: test.hex


FORMAT = ihex 		# create a .hex file

OPT = s			# assembly-level optimization

# Optional compiler flags.
#  -g:        generate debugging information (for GDB, or for COFF conversion)
#  -O*:       optimization level
#  -f...:     tuning, see gcc manual and avr-libc documentation
#  -Wall...:  warning level
#  -Wa,...:   tell GCC to pass this to the assembler.
#    -ahlms:  create assembler listing
CFLAGS = -g -O$(OPT) \
-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums \
-Wall -Wextra -Werror \
-DF_CPU=$(F_CPU) \
-D__$(MCU)__ \
-DWITH_URL2 \
-DWITH_WATCHDOG \
-Wa,-adhlns=$(<:.c=.lst) \
$(patsubst %,-I%,$(EXTRAINCDIRS)) \
-mmcu=$(MCU)

# Set a "language standard" compiler flag.
CFLAGS += -std=gnu99

#================= Build Options =====================

# Set target environment (URL).
ifeq ("$(TARGET)", "")
  # header file decides URl
else ifeq ($(TARGET), DEMO)
  CFLAGS += -DURL='"http://nfcofflineconversion.appspot.com"'
else
  $(error unknown TARGET)
endif

# LCD is off by default.
ifdef HAS_LCD
  DEBUG_OBJS += peripheral/lcd.o
  CFLAGS += -DHAS_LCD
endif

# Reset device if external power is plugged in or out. Useful for device
# with built-in battery charger.
ifdef HAS_CHARGER
  CFLAGS += -DHAS_CHARGER
endif

# Do not include real phone IDM in URL
ifdef WITH_FAKE_IDM
  CFLAGS += -DFAKE_IDM
endif

# Optional assembler flags.
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information; note that
#             for use in COFF files, additional information about filenames
#             and function names needs to be present in the assembler source
#             files -- see avr-libc docs [FIXME: not yet described there]
ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs


# Optional linker flags.
#  -Wl,...:   tell GCC to pass this to linker.
#  -Map:      create map file
#  --cref:    add cross reference to  map file
LDFLAGS = -Wl,-Map=$(TARGET).map,--cref

# ---------------------------------------------------------------------------

# Define directories, if needed.
DIRAVR = c:/winavr
DIRAVRBIN = $(DIRAVR)/bin
DIRAVRUTILS = $(DIRAVR)/utils/bin
DIRINC = .
DIRLIB = $(DIRAVR)/avr/lib


# Define programs and commands.
SHELL = sh

CC = avr-gcc

OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size

REMOVE = rm -f
COPY = cp

# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = -------- begin --------
MSG_END = --------  end  --------
MSG_SIZE = Binary size: 
MSG_FLASH = Creating load file for Flash:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling:
MSG_ASSEMBLING = Assembling:
MSG_CLEANING = Cleaning project:

# Define all object files.
OBJ = $(SRC:.c=.o) $(ASRC:.S=.o)

# Define all listing files.
LST = $(ASRC:.S=.lst) $(SRC:.c=.lst)

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -I. $(CFLAGS)
ALL_ASFLAGS = -mmcu=$(MCU) -I. -x assembler-with-cpp $(ASFLAGS)

#================= Text Output =====================

# AVR Studio 3.x does not check make's exit code but relies on
# the following magic strings to be generated by the compile job.
begin:
	@echo
	@echo $(MSG_BEGIN)

finished:
	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
	@echo

# Display compiler version information.
gccversion :
	@$(CC) --version


#================= Binary Image Rules =====================

station-active.elf: $(STATION_OBJS) $(LIB_OBJS) $(RCS956_OBJS) $(DEBUG_OBJS)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(ALL_CFLAGS) $^ --output $@ $(LDFLAGS)

station-activepassive.elf: $(STATION_OBJS) $(LIB_OBJS) $(RCS956_OBJS) \
	                   $(TARGET_OBJS) $(DEBUG_OBJS)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(ALL_CFLAGS) $^ --output $@ $(LDFLAGS)

station-plug.elf: $(PLUG_OBJS) $(LIB_OBJS) $(RCS926_OBJS) $(DEBUG_OBJS)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(ALL_CFLAGS) $^ --output $@ $(LDFLAGS)

test.elf: $(TEST_OBJS) $(LIB_OBJS)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(ALL_CFLAGS) $^ --output $@ $(LDFLAGS)

# Create final output files (.hex) from ELF output file.
%.hex: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@
	@avr-size $<

%.eep: %.elf
	@echo
	@echo $(MSG_EEPROM) $@
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
		--change-section-lma .eeprom=0 --no-change-warnings -O \
		$(FORMAT) $< $@ || exit 0

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJ)


#================= Compile Rules =====================

# Compile: create object files from C source files.
%.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(ALL_CFLAGS) $< -o $@


# Compile: create assembler files from C source files.
%.s : %.c
	$(CC) -S $(ALL_CFLAGS) $< -o $@


# Assemble: create object files from assembler source files.
%.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@

#================= Utility Targets =====================

# Program Fuses (includes BOD = 2.7V)
fuse_osc_lo:
	hidspx -d9 -fL0xCC -fX0xFD

fuse_osc_hi:
	hidspx -d9 -fL0xC7 -fX0xFD

fuse_quartz:
	hidspx -d9 -fL0xE7 -fX0xFD

# Target: clean project.
clean: begin clean_list finished end

clean_list :
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) *.hex
	$(REMOVE) *.elf
	$(REMOVE) $(EEP)
	$(REMOVE) $(ALL_OBJS)
	$(REMOVE) $(subst .o,.lst,$(ALL_OBJS))

# Listing of phony targets.
.PHONY : all begin finish end \
	clean clean_list program
