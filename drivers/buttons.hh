// Copyright 2016 Matthias Puech.
//
// Author: Matthias Puech <matthias.puech@gmail.com>
// Based on code by: Olivier Gillet <ol.gillet@gmail.com>
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
// Driver for the front panel buttons.

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "stmlib/stmlib.h"

#include <stm32f4xx_conf.h>
#include <algorithm>

using namespace std;

const uint8_t kNumButtons = 8;

enum ButtonNames {
  BUTTON_1,
  BUTTON_2,
  BUTTON_3,
  BUTTON_4,
  BUTTON_5,
  BUTTON_6,
  BUTTON_REPEAT,
  BUTTON_DELETE,
};

struct PinAssign {
	GPIO_TypeDef* gpio;
	uint16_t pin;
};

const PinAssign button_pins[kNumButtons] = {
  {GPIOD, GPIO_Pin_2},
  {GPIOD, GPIO_Pin_4},
  {GPIOG, GPIO_Pin_9},
  {GPIOG, GPIO_Pin_11},
  {GPIOG, GPIO_Pin_13},
  {GPIOB, GPIO_Pin_8},
  {GPIOC, GPIO_Pin_11},
  {GPIOA, GPIO_Pin_15},
};

class Buttons {
 public:
  void Init() {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Mode = GPIO_Mode_IN;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_25MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;

    for (uint8_t i=0; i<kNumButtons; i++) {
      gpio.GPIO_Pin = button_pins[i].pin;
      GPIO_Init(button_pins[i].gpio, &gpio);
    }

    fill(&button_state_[0], &button_state_[kNumButtons], 0xff);

    for (int i=0; i<32; i++) {
      Debounce();
      for (int j=0; j<100000; j++)
        __asm__ __volatile__("nop"); // delay
    }
  }

  void Debounce() {
    for (uint8_t i=0; i<kNumButtons; i++) {
      button_state_[i] = (button_state_[i] << 1) | \
        GPIO_ReadInputDataBit(button_pins[i].gpio, button_pins[i].pin);
    }
  }

  inline bool released(uint8_t index) const {
    return button_state_[index] == 0x7f;
  }
  
  inline bool just_pressed(uint8_t index) const {
    return button_state_[index] == 0x80;
  }

  inline bool pressed(uint8_t index) const {
    return button_state_[index] == 0x00;
  }


private:
  uint8_t button_state_[kNumButtons];
};

#endif
