#pragma once
#include "GateInChecker.h"
#include "drivers/dac.hh"
#include "drivers/gate_input.hh"
#include "drivers/leds.hh"
#include "hardwaretests/utils.hh"

class TapoGateInChecker : public IGateInChecker {
	GateInput gate;
	DigOut out;
	Leds &leds;

public:
	TapoGateInChecker(Leds &leds)
		: IGateInChecker(1)
		, leds{leds} {
		gate.Init();
		out.Init();
	}

	virtual ~TapoGateInChecker() {}

	virtual bool read_gate(uint8_t gate_num) {
		gate.Read();
		return gate.value(0);
	}

	virtual void set_test_signal(bool newstate) {
		out.Write(newstate);
		TapoHWTestUtil::delay_ms(100);
	}

	virtual void set_error_indicator(uint8_t channel, ErrorType err) {
		if (err==ErrorType::None)
			return;
		leds.set_delete(COLOR_YELLOW);
		leds.set_repeat(COLOR_YELLOW);
		leds.Write();
	}
	virtual void set_indicator(uint8_t indicate_num, bool newstate) {
		leds.set_repeat(newstate? COLOR_GREEN : COLOR_BLACK);
		leds.Write();
	}
	virtual void signal_jack_done(uint8_t chan) {
		//
	}
	virtual bool is_ready_to_read_jack(uint8_t chan) {
		return true;
	}
};
