#include "audio_tester.hh"
#include "CodecCallbacks.hh"
#include "drivers/codec.hh"
#include "drivers/leds.hh"

// FixMe: Tapo startup does not call static constructors. This does not call
// ctor:
// SkewedTriOsc osc0{500, 0.6, 32767, -32768, 0, 48000};
// SkewedTriOsc osc1{2000, 0.6, 32767, -32768, 0, 48000};

namespace {
SkewedTriOsc osc0;
bool highnote = false;
uint32_t ctr = 100;
} // namespace

void TapoAudioChecker::reset() {
  osc0.init(20000, 0.8, 32767, -32768, 0, 48000);
}

void TapoAudioChecker::osc_test_cb(Frame *rx, Frame *tx) {
  for (int i = 0; i < CODEC_BUFFER_SIZE; i++) {
    tx[i].l = (int16_t)(osc0.update());
    tx[i].r = rx[i].l;
  }
  if (!ctr--) {
    ctr = 36;
    highnote = !highnote;
    osc0.init(highnote ? 20000 : 20, 0.8, 32767, -32768, 0.5, 48000);
  }
}
