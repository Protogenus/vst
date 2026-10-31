// LowpassFilter.h — TPT 2-pole state-variable lowpass (Zavalishin), cascadable to 4-pole.
// Stable at all cutoffs, no denormals (tiny DC offset not needed; states decay).
#pragma once
#include <cmath>
#include "DspCommon.h"

namespace absynth {

class LowpassStage {
public:
    void setSampleRate(double sr) { sampleRate = sr; update(); }
    void setCutoff(double hz) {
        // clamp to sane range; never exceed Nyquist*0.49
        double maxF = sampleRate * 0.49;
        cutoff = hz < 20.0 ? 20.0 : (hz > maxF ? maxF : hz);
        update();
    }
    void setQ(double q) { Q = q < 0.5 ? 0.5 : (q > 8.0 ? 8.0 : q); update(); }

    double process(double x) {
        // TPT SVF lowpass
        double v0 = x;
        double v3 = v0 - ic2;
        double v1 = a1 * ic1 + a2 * v3;
        double v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0 * v1 - ic1;
        ic2 = 2.0 * v2 - ic2;
        return v2;
    }

    void reset() { ic1 = ic2 = 0.0; }

private:
    void update() {
        double g = std::tan(kPi * cutoff / sampleRate);
        double k = 1.0 / Q;
        a1 = 1.0 / (1.0 + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }
    double sampleRate = 48000.0, cutoff = 2000.0, Q = 0.7071;
    double a1 = 0, a2 = 0, a3 = 0, ic1 = 0, ic2 = 0;
};

class LowpassFilter {
public:
    void setSampleRate(double sr) { s1.setSampleRate(sr); s2.setSampleRate(sr); }
    void setCutoff(double hz) { s1.setCutoff(hz); s2.setCutoff(hz); }
    void setQ(double q) { s1.setQ(q); s2.setQ(q); }
    void setPoles(int poles24) { use24 = (poles24 != 0); } // true = 24dB/oct cascade

    double process(double x) {
        double y = s1.process(x);
        return use24 ? s2.process(y) : y;
    }
    void reset() { s1.reset(); s2.reset(); }

private:
    LowpassStage s1, s2;
    bool use24 = true;
};

} // namespace absynth
