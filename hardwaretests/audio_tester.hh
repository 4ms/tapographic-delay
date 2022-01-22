#pragma once
#include "CodecCallbacks.hh"
#include "drivers/codec.hh"
#include "drivers/leds.hh"
#include "hardwaretests/utils.hh"

class TapoAudioChecker {
	Leds &leds;
	Codec &codec;

public:
	TapoAudioChecker(Leds &leds, Codec &codec)
		: leds(leds)
		, codec(codec) {}

	void reset();

	void run_test() {
		reset();

		leds.Clear();
		leds.set_rgb(1, COLOR_BLUE);
		leds.Write();

		Codec codec;
		leds.set_rgb(2, COLOR_BLUE);
		leds.Write();

		bool init_ok = codec.Init(48000, osc_test_cb);
		leds.set_rgb(3, COLOR_BLUE);
		leds.Write();

		if (!init_ok) {
			while (1) {
				leds.set_rgb(3, COLOR_RED);
				leds.Write();
				TapoHWTestUtil::delay_ms(100);

				leds.set_rgb(3, COLOR_BLACK);
				leds.Write();
				TapoHWTestUtil::delay_ms(100);
			}
		}
		TapoHWTestUtil::flash_mainbut_until_pressed();

		codec.Stop();
	}

	static void osc_test_cb(Frame *rx, Frame *tx);
};
