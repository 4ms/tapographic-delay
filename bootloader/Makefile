# Copyright 2016 Matthias Puech.
#
# Author: Matthias Puech <matthias.puech@gmail.com>
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

# System specifications
F_CRYSTAL      = 8000000L
F_CPU          = 180000000L
SYSCLOCK       = SYSCLK_FREQ_180MHz
FAMILY         = f4xx
MCU            = STM32F427_437xx -DHAS_FMC
SAMPLE_RATE    = 48000

# Preferred upload command
UPLOAD_COMMAND  = upload_jtag

# Packages to build
TARGET         = tapo_bootloader
PACKAGES       = bootloader drivers \
		 stmlib/utils stmlib/system \
		 stm_audio_bootloader/qpsk

include stmlib/makefile.inc
