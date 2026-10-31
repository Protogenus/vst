// Pump.h — rhythmic gain pump (sidechain-style breathing). Fixed state, no allocation.
#pragma once
#include <cmath>
#include "DspCommon.h"
#include "Smooth.h"

namespace absynth {

class Pump {
public:
    void setSampleRate(double sr) {
        sampleRate = sr;
        amountSm.setSampleRate(sr); amountSm.setTimeMs(30);
        updateInc();
    }
    void setAmount(double a01) { amountSm.setTarget(a01 < 0 ? 0 : (a01 > 1 ? 1 : a01)); }
    void setRate(double hz) { rate = hz < 0.2 ? 0.2 : (hz > 12.0 ? 12.0 : hz); updateInc(); }
    void snap() { amountSm.snapTo(amountSm.get()); }

    void process(double& l, double& r) {
        double amount = amountSm.process();
        // Sharp dip, smooth recovery — like a kick ducking the bus.
        double s = 0.5 + 0.5 * std::sin(phase);
        double dip = s * s * s;
        double g = 1.0 - amount * 0.85 * dip;
        l *= g; r *= g;
        phase += inc;
        if (phase >= 2.0 * kPi) phase -= 2.0 * kPi;
    }

    void reset() { phase = 0.0; }

private:
    void updateInc() { inc = 2.0 * kPi * rate / sampleRate; }
    double sampleRate = 48000.0;
    double rate = 2.2, phase = 0.0, inc = 0.0;
    Smoother amountSm;
};

} // namespace absynth
