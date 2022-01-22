#pragma once
#include "drivers/adc.hh"
#include "drivers/buttons.hh"
#include "drivers/codec.hh"
#include "drivers/gate_input.hh"
#include "drivers/leds.hh"
#include "drivers/switches.hh"

struct TapoHardwareTester {
	void run_if_requested() {
		if (check_should_run())
			run();
	}

private:
	Buttons buttons;
	Leds leds;
	Codec codec;

	bool check_should_run();
	void run();

	void test_audio();
	void test_leds();
	void test_buttons();
	void test_switches();
	void test_adc();
	void test_RAM();
	void test_gate_in_jacks();
	void success_animation(uint16_t animation_ctr) ;
};
