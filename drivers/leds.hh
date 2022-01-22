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
// Driver for the status LEDs.

#ifndef LEDS_H_
#define LEDS_H_

// activate to enable prototype 2 compatibility mode
// #define PROTO2

#include "stmlib/stmlib.h"

#include <stm32f4xx_conf.h>

const uint8_t kNumLeds = 26;

enum LedColor {
  COLOR_BLACK,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_YELLOW,
  COLOR_BLUE,
  COLOR_MAGENTA,
  COLOR_CYAN,
  COLOR_WHITE,
};

enum LedNames {
  LED_BUT1_R,
  LED_BUT1_G,
  LED_BUT1_B,
  LED_BUT2_R,
  LED_BUT2_G,
  LED_BUT2_B,
  LED_BUT3_R,
  LED_BUT3_G,
  LED_BUT3_B,
  LED_BUT4_R,
  LED_BUT4_G,
  LED_BUT4_B,
  LED_BUT5_R,
  LED_BUT5_G,
  LED_BUT5_B,
  LED_BUT6_R,
  LED_BUT6_G,
  LED_BUT6_B,
  LED_DELETE_R,
  LED_DELETE_G,
  LED_DELETE_B,
  LED_REPEAT_R,
  LED_REPEAT_G,
  LED_REPEAT_B,
  LED_TAP,
  OUT_VELNORM,                  // normalization for Vel (not an LED)
};

static uint16_t const LED_Pins[kNumLeds] = {
    GPIO_Pin_6,
    GPIO_Pin_13,
    GPIO_Pin_14,
    GPIO_Pin_2,
		GPIO_Pin_5,
    GPIO_Pin_5,
    GPIO_Pin_13,
		GPIO_Pin_12,
		GPIO_Pin_11,
    GPIO_Pin_7,
		GPIO_Pin_6,
		GPIO_Pin_3,
    GPIO_Pin_9,
		GPIO_Pin_8,
		GPIO_Pin_7,
		GPIO_Pin_10,
    GPIO_Pin_9,
    GPIO_Pin_8,
    GPIO_Pin_10,
    GPIO_Pin_3,
    GPIO_Pin_7,
    GPIO_Pin_12,
    GPIO_Pin_10,
    GPIO_Pin_12,
    GPIO_Pin_0,
    GPIO_Pin_9,
};

static uint16_t const LED_PinSources[kNumLeds] = {
    GPIO_PinSource6,
    GPIO_PinSource13,
    GPIO_PinSource14,
    GPIO_PinSource2,
		GPIO_PinSource5,
    GPIO_PinSource5,
    GPIO_PinSource13,
		GPIO_PinSource12,
		GPIO_PinSource11,
    GPIO_PinSource7,
		GPIO_PinSource6,
		GPIO_PinSource3,
    GPIO_PinSource9,
		GPIO_PinSource8,
		GPIO_PinSource7,
		GPIO_PinSource10,
    GPIO_PinSource9,
    GPIO_PinSource8,
    GPIO_PinSource10,
    GPIO_PinSource3,
    GPIO_PinSource7,
    GPIO_PinSource12,
    GPIO_PinSource10,
    GPIO_PinSource12,
    GPIO_PinSource0,
    GPIO_PinSource9,
};

static GPIO_TypeDef* const LED_GPIOs[kNumLeds] = {
    GPIOE,
		GPIOC,
    GPIOC,
    GPIOB,
		GPIOC,
		GPIOE,
    GPIOD,
		GPIOD,
		GPIOD,
    GPIOG,
		GPIOG,
		GPIOG,
    GPIOC,
		GPIOC,
		GPIOC,
		GPIOA,
		GPIOA,
    GPIOA,
    GPIOC,                      // del R
    GPIOD,                      // del G
    GPIOD,                      // del B
    GPIOC,                      // rep R
    GPIOG,                      // rep G
    GPIOG,                      // rep B
    GPIOA,                      // tap
    GPIOB,                      // vel norm
};

class Leds {
 public:
  void Init() {

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA |
                           RCC_AHB1Periph_GPIOB |
                           RCC_AHB1Periph_GPIOC |
                           RCC_AHB1Periph_GPIOD |
                           RCC_AHB1Periph_GPIOE |
                           RCC_AHB1Periph_GPIOG, ENABLE);

    GPIO_InitTypeDef gpio;
    GPIO_StructInit(&gpio);
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;

    // LED pins configure
    for (int i=0; i<kNumLeds; i++) {
      gpio.GPIO_Pin = LED_Pins[i];
      GPIO_Init(LED_GPIOs[i], &gpio);
    }

    Clear();
    Write();
  }

  void Write() {
    for (int i=0; i<kNumLeds; i++) {
      bool v = i<18 ? !values_[i] : values_[i];
      GPIO_WriteBit(LED_GPIOs[i], LED_Pins[i], static_cast<BitAction>(v));
    }
  }

  void Clear() {
    for (int i=0; i<kNumLeds; i++) {
      set(i, false);
    }
  }

  void set(uint8_t channel, bool value) {
#ifdef PROTO2
    if (channel == OUT_VELNORM ||
        channel == LED_REPEAT_G ||
        channel == LED_REPEAT_B ||
        channel == LED_DELETE_G ||
        channel == LED_DELETE_B)
      value = false;
    values_[channel] = value;
# else
    if (channel >= LED_DELETE_R &&
        channel <= LED_REPEAT_B) {
      values_[channel] = !value;
    } else {
      values_[channel] = value;
    }
#endif
  }

  void set_rgb(uint8_t channel, uint8_t color) {
    for (int i=0; i<3; i++) {
      set(channel * 3 + i, (color >> i) & 1);
    }
  }

  void set_repeat(uint8_t color) {
    set(LED_REPEAT_R, (color >> 0) & 1);
    set(LED_REPEAT_G, (color >> 1) & 1);
    set(LED_REPEAT_B, (color >> 2) & 1);
  }

  void set_delete(uint8_t color) {
    set(LED_DELETE_R, (color >> 0) & 1);
    set(LED_DELETE_G, (color >> 1) & 1);
    set(LED_DELETE_B, (color >> 2) & 1);
  }

 private:
  bool values_[kNumLeds];
};

#endif
