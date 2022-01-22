// Copyright 2015 Matthias Puech.
//
// Original Author: Olivier Gillet (ol.gillet@gmail.com)
// Modified by: Matthias Puech (matthias.puech@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// System-level initialization.

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "misc.h"
#include "stmlib/stmlib.h"
#include "system_stm32f4xx.h"

class System {
 public:

  void Init(bool application) {
    SystemInit();
    if (application) {
      NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000);
    }
  }

  void StartTimers() {
    SysTick_Config(F_CPU / 1000);
  }

 
 private:
};

#endif
