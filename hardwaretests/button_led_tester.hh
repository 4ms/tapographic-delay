#pragma once
#include "ButtonChecker.h"
#include "drivers/buttons.hh"
#include "drivers/leds.hh"
#include "hardwaretests/utils.hh"

static constexpr int NumButtons = 8;

class TapoButtonChecker : public IButtonChecker {
public:
	Buttons &buttons;
	Leds &leds;

	TapoButtonChecker(Buttons &buttons, Leds &leds)
		: IButtonChecker(NumButtons)
		, buttons{buttons}
		, leds{leds} {}
	~TapoButtonChecker() = default;

	//FixMe: Why is this required?
	void operator delete(void *, unsigned int) {}

	virtual bool _read_button(uint8_t channel) {
        return !GPIO_ReadInputDataBit(button_pins[channel].gpio, button_pins[channel].pin);
	}

	virtual void _set_error_indicator(uint8_t channel, ErrorType err) {
		switch (err) {
			case ErrorType::None:
				leds.set_rgb(channel, COLOR_BLACK);
				leds.Write();
				break;
			default:
				for (int i = 0; i < 10; i++) {
					leds.set_rgb(channel, COLOR_RED);
					leds.Write();
					TapoHWTestUtil::delay_ms(100);
					leds.set_rgb(channel, COLOR_BLACK);
					leds.Write();
					TapoHWTestUtil::delay_ms(100);
				}
				break;
		}
	}

	virtual void _set_indicator(uint8_t indicator_num, bool newstate) {
		//Button and LED numbering swap DELETE and REPEAT
		if (indicator_num == 6)
			indicator_num = 7;
		else if (indicator_num == 7)
			indicator_num = 6;

		leds.set_rgb(indicator_num, newstate ? COLOR_WHITE : COLOR_BLACK);
		leds.Write();
	}
	virtual void _check_max_one_pin_changed() {}
};
