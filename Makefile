# Copyright 2017 Matthias Puech.
#
# Author: Matthias Puech (matthias.puech@gmail.com)
# Based on code by: Olivier Gillet <ol.gillet@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# See http://creativecommons.org/licenses/MIT/ for more information.
#
# -----------------------------------------------------------------------------
#
# Makefile driver.

TOOLCHAIN_PATH ?= /usr/local/

# System specifications
F_CRYSTAL      = 8000000L
F_CPU          = 180000000L
SYSCLOCK       = SYSCLK_FREQ_180MHz
FAMILY         = f4xx
MCU            = STM32F427_437xx -DHAS_FMC
SAMPLE_RATE    = 48000

APPLICATION_LARGE    = TRUE

# Preferred upload command
UPLOAD_COMMAND  = upload_combo_jtag

# Packages to build
TARGET         = tapo
BOOTLOADER     = tapo_bootloader
PACKAGES       = . drivers stmlib/utils stmlib/system stmlib/dsp libhwtests/src hardwaretests
RESOURCES      = resources
INCLUDEFLAGS   = -I libhwtests/inc -I hardwaretests
EXTRA_CPP_FLAGS = -Wno-register

PGM_INTERFACE = stlink-v2-1-swd
PGM_INTERFACE_TYPE = hla-swd

include stmlib/makefile.inc

wav:  $(TARGET_BIN)
	# Using macOS built-in python2
	/usr/bin/python2 stm_audio_bootloader/qpsk/encoder.py \
		-t stm32f4 -s 48000 -b 12000 -c 6000 -p 256 \
		$(TARGET_BIN)

compile_commands:
	compiledb make
	compdb -p ./ list > compile_commands_with_headers.json 2>/dev/null
	rm compile_commands.json 
	mv compile_commands_with_headers.json compile_commands.json

