#include "AdcChecker.hh"
#include "CodecCallbacks.hh"
#include "drivers/adc.hh"
#include "drivers/codec.hh"
#include "drivers/dac.hh"
#include "drivers/leds.hh"
#include "hardwaretests/utils.hh"
#include "stm32f4xx_dac.h"

class TapoAdcChecker : public IAdcChecker {
	Leds &leds;
	Codec &codec;
	Adc adc_;

	static constexpr uint8_t num_pots = ADC_GAIN_POT - ADC_SCALE_POT + 1;
	static constexpr uint8_t num_bipolarCV = ADC_DRYWET_CV - ADC_SCALE_CV + 1;
	static constexpr uint8_t num_unipolarCV = ADC_TAPTRIG_CV - ADC_CLOCK_CV + 1;
	static constexpr uint8_t num_adc = num_pots + num_bipolarCV + num_unipolarCV;

	static constexpr AdcRangeCheckerBounds bounds = {
		.center_val = 2048,
		.center_width = 200,
		.center_check_counts = 400,
		.min_val = 350,
		.max_val = 4080,
	};

public:
	TapoAdcChecker(Leds &leds, Codec &codec)
		: IAdcChecker{bounds, num_pots, num_bipolarCV, num_unipolarCV}
		, leds{leds}
		, codec{codec} {
	}

	void run_test() {
		for (int i = 0; i < 6; i++)
			leds.set_rgb(i, COLOR_BLACK);
		leds.Write();

		_setup_adc();
		_setup_test_signals();

		IAdcChecker::run_test();

		dac.DeInit();
		codec.Stop();
	}

private:
	void _setup_adc() {
		leds.set_repeat(COLOR_RED);
		leds.set_delete(COLOR_WHITE);
		leds.Write();
		adc_.DeInit();
		adc_.Init();
		leds.set_repeat(COLOR_RED);
		leds.set_delete(COLOR_BLUE);
		leds.Write();
	}

	void _setup_test_signals() {
		bipolar_test_signal.init(4, 0.6, (8.0f / FS_max) * 32767, (-8.0f / FS_min) * -32768, 0, 48000);
		unipolar_test_signal.init(2, 0.2, 4000, 0, 0, 48000.f / (float)CODEC_BUFFER_SIZE);

		dac.Init();
		bool init_ok = codec.Init(48000, TapoAdcChecker::lfo_test_signals);
		if (!init_ok) {
			while (1) {
				leds.set_repeat(COLOR_YELLOW);
				leds.set_delete(COLOR_RED);
				leds.Write();
				TapoHWTestUtil::delay_ms(100);
				leds.set_delete(COLOR_BLACK);
				leds.set_repeat(COLOR_BLACK);
				leds.Write();
				TapoHWTestUtil::delay_ms(100);
			}
		}
		leds.set_delete(COLOR_BLACK);
		leds.set_repeat(COLOR_BLACK);
		leds.Write();
	}

	virtual void set_indicator(uint8_t adc_i, AdcType adctype, AdcCheck_State state) override {
		// auto led_i = adctype == Pot		   ? adc_i :
		// 			 adctype == BipolarCV  ? adc_i :
		// 			 adctype == UnipolarCV ? adc_i :
		// 									   0;
		switch (state) {
			case ADCCHECK_NO_COVERAGE:
				leds.set_rgb(0, COLOR_RED);
				leds.set_rgb(1, COLOR_RED);
				leds.set_rgb(2, COLOR_BLUE);
				leds.set_rgb(3, COLOR_BLUE);
				leds.set_rgb(4, COLOR_GREEN);
				leds.set_rgb(5, COLOR_GREEN);
				break;

			case ADCCHECK_AT_MIN:
				leds.set_rgb(0, COLOR_BLACK);
				leds.set_rgb(1, COLOR_BLACK);
				break;

			case ADCCHECK_AT_MAX:
				leds.set_rgb(4, COLOR_BLACK);
				leds.set_rgb(5, COLOR_BLACK);
				break;

			case ADCCHECK_AT_CENTER:
				leds.set_rgb(2, COLOR_BLACK);
				leds.set_rgb(3, COLOR_BLACK);
				break;

			case ADCCHECK_ELSEWHERE:
				leds.set_rgb(2, COLOR_BLUE);
				leds.set_rgb(3, COLOR_BLUE);
				break;

			case ADCCHECK_FULLY_COVERED:
				for (int i = 0; i < 6; i++)
					leds.set_rgb(i, COLOR_GREEN);
				leds.Write();
				delay_ms(50);
				for (int i = 0; i < 6; i++)
					leds.set_rgb(i, COLOR_BLACK);
				break;
		}
		leds.Write();
	}

	virtual uint32_t get_adc_reading(uint8_t adc_i, AdcType adctype) override {
		uint32_t val = 0;
		constexpr float FSR_ADJUSTMENT = 4095.f / 3200.f;
		constexpr uint8_t pot_map[6] = {5, 0, 1, 2, 3, 4};
		constexpr uint8_t unicv_map[4] = {3, 2, 0, 1};
		if (adc_i < ADC_CHANNEL_LAST) {
			if (adctype == Pot)
				val = adc_.value(pot_map[adc_i]) >> 4;
			if (adctype == BipolarCV)
				val = adc_.value(adc_i + ADC_SCALE_CV) >> 4;
			if (adctype == UnipolarCV) {
				val = adc_.value(unicv_map[adc_i] + ADC_CLOCK_CV) >> 4;
				if (adc_i == 3)
					val = (float)val * FSR_ADJUSTMENT;
			}
		}
		adc_.Convert();
		return val;
	}

	virtual void pause_between_steps() override {
		TapoHWTestUtil::pause_until_button_released();
		TapoHWTestUtil::delay_ms(100);
	}

	virtual bool button_to_skip_step() override {
		if (TapoHWTestUtil::main_button_pressed()) {
			set_indicator(0, Pot, ADCCHECK_FULLY_COVERED);
			return true;
		} else
			return false;
	}

	//Flash Delete Red/Yellow while Repeat is Red... until you press Delete to continue
	virtual void show_multiple_nonzeros_error() override {
		leds.set_delete(COLOR_RED);
		leds.set_repeat(COLOR_RED);
		leds.Write();
		TapoHWTestUtil::flash_mainbut_until_pressed();
		leds.set_delete(COLOR_BLACK);
		leds.set_repeat(COLOR_BLACK);
		leds.Write();
	}

	virtual void delay_ms(uint32_t x) override {
		TapoHWTestUtil::delay_ms(x);
	}

private:
	static constexpr float FS_min = -10.0f;
	static constexpr float FS_max = 10.0f;
	static inline CenterFlatRamp bipolar_test_signal;
	static inline CenterFlatRamp unipolar_test_signal;
	static Dac dac;

	static void lfo_test_signals(Frame *rx, Frame *tx) {
		dac.WriteInt((uint16_t)(unipolar_test_signal.update()));
		for (int i = 0; i < CODEC_BUFFER_SIZE; i++) {
			tx[i].l = (int16_t)(bipolar_test_signal.update());
			tx[i].r = tx[i].l;
		}
	}
};
