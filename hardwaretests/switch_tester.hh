#pragma once
#include "SwitchChecker.hh"
#include "drivers/leds.hh"
#include "drivers/switches.hh"
#include "hardwaretests/utils.hh"

constexpr unsigned Num3posSwitches = 2;

class TapoSwitchChecker : public ISwitchChecker<Num3posSwitches> {
	Leds &leds;

public:
	TapoSwitchChecker(Leds &leds)
		: leds{leds} {
		switches.Init();
		for (unsigned i = 0; i < Num3posSwitches * 3; i++)
			leds.set(i, true);
	}

	virtual SwitchPosition read_switch_state(uint32_t sw_num) {
		switches.Read();
		uint8_t state = switches.state(sw_num);
		return state == 0b10 ? SwitchPosition::Up :
			   state == 0b01 ? SwitchPosition::Down :
			   state == 0b00 ? SwitchPosition::Middle :
								 SwitchPosition::Unknown;
	}

	virtual void delay_ms(uint32_t ms) {
		TapoHWTestUtil::delay_ms(ms);
	}

	virtual void set_indicator(uint32_t sw_num, SwitchPosition pos) {
		if (pos == SwitchPosition::Initial)
			leds.set_rgb(sw_num, COLOR_WHITE);
		else {
			uint8_t led_id = sw_num * 3 + static_cast<uint8_t>(pos);
			leds.set(led_id, false);
		}
		leds.Write();
	}
	virtual void set_error_indicator(uint32_t sw_num) {
		leds.set_rgb(4 + sw_num, COLOR_RED);
	}

private:
	Switches switches;
};

