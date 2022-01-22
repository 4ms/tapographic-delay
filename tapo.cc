// Copyright 2015 Matthias Puech.
//
// Author: Matthias Puech (matthias.puech@gmail.com)
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
// Multitap delay, main file

#include <stm32f4xx_conf.h>

#include "stmlib/system/system_clock.h"
#include "drivers/system.hh"
#include "drivers/codec.hh"
#include "drivers/sdram.hh"
#include "drivers/dac.hh"
#include "ui.hh"
#include "multitap_delay.hh"
#include "hardware_tests.hh"

using namespace stmlib;

System sys;
SDRAM sdram;
Ui ui;
MultitapDelay delay;
DigOut dac;
Codec codec;

Parameters parameters;

bool Panic() {
  codec.Stop();
  ui.Panic();
  while(1);
  return true;
}

extern "C" {
  void NMI_Handler() { }
  void HardFault_Handler() { while (1); }
  void MemManage_Handler() { while (1); }
  void BusFault_Handler() { while (1); }
  void UsageFault_Handler() { while (1); }
  void SVC_Handler() { }
  void DebugMon_Handler() { }
  void PendSV_Handler() { }
  void assert_failed(uint8_t* file, uint32_t line) { while (1); }

  // slow timer for the UI
  void SysTick_Handler() {
    ui.Poll();
    system_clock.Tick();  // increment global ms counter.
  }
  
  void FillBuffer(Frame* input, Frame* output) {
    ui.ReadParameters();
    // dac.Write(true);            // profiling
    delay.Process(&parameters, (ShortFrame*)input, (ShortFrame*)output);
    // dac.Write(false);           // profiling
    dac.Update();
  }

  void ping_gate_out() {
    dac.Ping();
    ui.PingGateLed();
  }
}

void Init() {
#ifdef APPLICATION
  sys.Init(true);
#else
  sys.Init(false);
#endif

  system_clock.Init();

  TapoHardwareTester tester;
  tester.run_if_requested();

  sdram.Init();
  dac.Init();
  delay.Init((short*)SDRAM_BASE, SDRAM_SIZE/sizeof(short) / 2);
  ui.Init(&delay, &parameters);
  sys.StartTimers();

  delay.tap_modulo_observable_.set_observer(&ping_gate_out);

  codec.Init(SAMPLE_RATE, &FillBuffer) || Panic();
}

int main(void) {
  Init();
  while(1) {
    ui.DoEvents();
    delay.Poll();
  }
}
