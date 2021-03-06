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
// Driver for the gate inputs.

#ifndef GATE_INPUT_H_
#define GATE_INPUT_H_

#include "stmlib/stmlib.h"

#include <stm32f4xx_conf.h>

enum GateNames {
  GATE_INPUT_REPEAT,
  GATE_INPUT_LAST
};

class GateInput {
 public:
  void Init() {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Mode = GPIO_Mode_IN;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
    gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;

    gpio_init.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOA, &gpio_init);

    for (int i=0; i<GATE_INPUT_LAST; i++) {
      previous_values_[i] = values_[i] = false;
    }
  }

  inline void Read() {
    for (int i=0; i<GATE_INPUT_LAST; i++) {
      previous_values_[i] = values_[i];
    }

    values_[GATE_INPUT_REPEAT] = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4);
  }

	inline bool value(int8_t channel) const { return values_[channel]; }

	inline bool rising_edge(int8_t channel) const {
		return values_[channel] && !previous_values_[channel];
  }
	inline bool falling_edge(int8_t channel) const {
		return !values_[channel] && previous_values_[channel];
  }

 private:
	bool previous_values_[GATE_INPUT_LAST];
	bool values_[GATE_INPUT_LAST];
};

#endif
