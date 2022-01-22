// Copyright 2016 Matthias Puech.
//
// Author: Matthias Puech <matthias.puech@gmail.com>
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
// Driver for DAC.

#ifndef DAC_H_
#define DAC_H_

#include "stmlib/stmlib.h"

#include <stm32f4xx_conf.h>

class DigOut {
public:
  void Init() {

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio_init.GPIO_Mode = GPIO_Mode_OUT;

    gpio_init.GPIO_Pin = GPIO_Pin_5;
    GPIO_Init(GPIOA, &gpio_init);

    counter_ = 0;
  }

  void Write(bool v) {
    GPIO_WriteBit(GPIOA, GPIO_Pin_5, static_cast<BitAction>(v));
  }

  uint16_t counter_;

  void Ping() { counter_ = 2; }

  void Update() {
    if (counter_ > 0) {
      Write(true);
      counter_--;
    } else {
      Write(false);
    }
  }

};

class Dac {
  public:
  void Init() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

	GPIO_InitTypeDef gpio_init;
	gpio_init.GPIO_Speed = GPIO_Speed_25MHz;
	gpio_init.GPIO_OType = GPIO_OType_PP;
	gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio_init.GPIO_Mode = GPIO_Mode_AF;
	gpio_init.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOA, &gpio_init);

	DAC_InitTypeDef dacinit{
		.DAC_Trigger = DAC_Trigger_None,
		.DAC_WaveGeneration = DAC_WaveGeneration_None,
		.DAC_LFSRUnmask_TriangleAmplitude = DAC_TriangleAmplitude_4095,
		.DAC_OutputBuffer = DAC_OutputBuffer_Enable,
	};
	DAC_Init(DAC_Channel_2, &dacinit);
	DAC_Cmd(DAC_Channel_2, ENABLE);
  }

  void Write(bool v) {
	  WriteInt(v ? 4095 : 0);
  }

  void WriteInt(uint16_t v) {
	DAC_SetChannel2Data(DAC_Align_12b_R, v );
  }

  uint16_t counter_;

  void Ping() { counter_ = 2; }

  void Update() {
    if (counter_ > 0) {
      Write(true);
      counter_--;
    } else {
      Write(false);
    }
  }

  void DeInit() {
    DAC_DeInit();
	DAC_Cmd(DAC_Channel_2, DISABLE);
  }

};
#endif
