#pragma once
#include "libhwtests/inc/HardwareTestUtil.hh"
#include "stm32f4xx_gpio.h"
#include <cstdint>

struct TapoHWTestUtil : IHardwareTestUtil<TapoHWTestUtil> {
	static inline GPIO_TypeDef *but_gpio = GPIOA;
	static inline const uint16_t but_pin = GPIO_Pin_15;

	static inline GPIO_TypeDef *but6_gpio = GPIOB;
	static inline const uint16_t but6_pin = GPIO_Pin_8;

	static inline GPIO_TypeDef *led_gpio = GPIOD;
	static inline const uint16_t led_pin = GPIO_Pin_3;

	static void init() {
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

		GPIO_InitTypeDef gpio;
		gpio.GPIO_Mode = GPIO_Mode_IN;
		gpio.GPIO_OType = GPIO_OType_PP;
		gpio.GPIO_Speed = GPIO_Speed_25MHz;
		gpio.GPIO_PuPd = GPIO_PuPd_UP;
		gpio.GPIO_Pin = but_pin;
		GPIO_Init(but_gpio, &gpio);

		gpio.GPIO_Pin = but6_pin;
		GPIO_Init(but6_gpio, &gpio);

		gpio.GPIO_Mode = GPIO_Mode_OUT;
		gpio.GPIO_OType = GPIO_OType_PP;
		gpio.GPIO_Speed = GPIO_Speed_2MHz;
		gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
		gpio.GPIO_Pin = led_pin;
		GPIO_Init(led_gpio, &gpio);
	}

	static void delay_ms(uint32_t x) {
		for (uint32_t i = 0; i < (130200 * x); ++i) {
			__asm__ __volatile__("nop\n\t" ::
								 : "memory");
		}
	}

	static bool main_button_pressed() {
		return !GPIO_ReadInputDataBit(but_gpio, but_pin);
	}

	static void set_main_button_led(bool turn_on) {
		GPIO_WriteBit(led_gpio, led_pin, turn_on ? Bit_SET : Bit_RESET);
	}

	static bool entry_button_pressed() {
		return !GPIO_ReadInputDataBit(but6_gpio, but6_pin);
	}
};
