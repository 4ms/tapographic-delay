#pragma once
#include "LEDTester.h"
#include "drivers/leds.hh"
#include "hardwaretests/utils.hh"

class TapoLEDChecker : public ILEDTester {
public:
	static constexpr int NumLEDs = 8 * 3 + 2;

	TapoLEDChecker(Leds &leds)
		: ILEDTester(NumLEDs)
		, leds{leds} {}

	Leds &leds;
	virtual void set_led(int led_id, bool turn_on) {
		leds.set(led_id, turn_on);
		leds.Write();
	}
	virtual void pause_between_steps() {
		uint32_t timeout = 1000000;

		while (timeout--) {
			if (!TapoHWTestUtil::main_button_pressed())
				timeout = 1000000;
		}
	}
};
