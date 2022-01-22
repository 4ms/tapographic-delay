#include "hardware_tests.hh"
#include "adc_tester.hh"
#include "audio_tester.hh"
#include "button_led_tester.hh"
#include "drivers/sdram.hh"
#include "hardwaretests/utils.hh"
#include "led_tester.hh"
#include "gate_in_tester.hh"
#include "switch_tester.hh"

#pragma GCC push_options
#pragma GCC optimize("-O1")

bool TapoHardwareTester::check_should_run() {
	TapoHWTestUtil::init();
	return TapoHWTestUtil::entry_button_pressed() && TapoHWTestUtil::main_button_pressed();
}

void TapoHardwareTester::run() {
	do {
		TapoHWTestUtil::set_main_button_led(true);
		TapoHWTestUtil::delay_ms(100);
		TapoHWTestUtil::set_main_button_led(false);
		TapoHWTestUtil::delay_ms(100);
	} while (TapoHWTestUtil::main_button_pressed());

	TapoHWTestUtil::delay_ms(100);

	leds.Init();
	buttons.Init();

	test_audio();
	test_leds();
	test_buttons();
	test_switches();
	test_adc();
	test_gate_in_jacks();
	test_RAM();

	uint16_t animation_ctr =0;
	while (1) {
		success_animation(animation_ctr++);
		TapoHWTestUtil::delay_ms(50);
	}
}

void TapoHardwareTester::test_audio() {
	TapoAudioChecker checker{leds, codec};
	checker.run_test();
}

void TapoHardwareTester::test_leds() {
	TapoLEDChecker checker{leds};
	checker.run_test();
}

void TapoHardwareTester::test_buttons() {
	TapoButtonChecker checker{buttons, leds};
	checker.reset();

	//Todo: adjust these values
	checker.set_min_steady_state_time(40000);
	checker.set_allowable_noise(100);

	bool is_running = true;
	while (is_running) {
		is_running = checker.check();
	}
}

void TapoHardwareTester::test_switches() {
	TapoSwitchChecker checker{leds};
	checker.run_test();
}

void TapoHardwareTester::test_adc() {
	TapoAdcChecker checker{leds, codec};
	checker.run_test();
}

void TapoHardwareTester::test_RAM() {
	SDRAM sdram;
	sdram.Init();

	leds.set_rgb(0, COLOR_YELLOW);	
	leds.set_rgb(1, COLOR_MAGENTA);	
	leds.set_rgb(2, COLOR_BLUE);	
	leds.Write();

	if (!sdram.Test()) {
		while(1) {
			leds.set_rgb(0, COLOR_RED);	
			leds.set_rgb(2, COLOR_RED);	
			leds.set_rgb(4, COLOR_RED);	
			leds.set_rgb(6, COLOR_RED);	
			leds.set_rgb(1, COLOR_BLACK);	
			leds.set_rgb(3, COLOR_BLACK);	
			leds.set_rgb(5, COLOR_BLACK);	
			leds.set_rgb(7, COLOR_BLACK);	
			leds.Write();
			TapoHWTestUtil::delay_ms(50);	
			leds.set_rgb(0, COLOR_BLACK);	
			leds.set_rgb(2, COLOR_BLACK);	
			leds.set_rgb(4, COLOR_BLACK);	
			leds.set_rgb(6, COLOR_BLACK);	
			leds.set_rgb(1, COLOR_RED);	
			leds.set_rgb(3, COLOR_RED);	
			leds.set_rgb(5, COLOR_RED);	
			leds.set_rgb(7, COLOR_RED);	
			leds.Write();
			TapoHWTestUtil::delay_ms(50);	
		}
	}
	leds.Clear();
	leds.Write();
}

void TapoHardwareTester::test_gate_in_jacks() {
	TapoGateInChecker checker{leds};
	checker.reset();
	while(checker.check());

}


void TapoHardwareTester::success_animation(uint16_t animation_ctr) {
    for (int i=0; i<6; i++) {
      int seed = animation_ctr / 4;
      LedColor c = static_cast<LedColor>((seed + i) % 5 + 2);
      if (((animation_ctr / 4) % 6 != i))
        c = COLOR_BLACK;

      leds.set_rgb(i, c);
    }
	leds.Write();
}

#pragma GCC pop_options
extern "C" void __cxa_pure_virtual() {}
