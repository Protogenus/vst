// SawOsc.h — polyBLEP anti-aliased sawtooth oscillator. Dependency-free C++17.
#pragma once
#include <cmath>

namespace absynth {

class SawOsc {
public:
    void setSampleRate(double sr) { sampleRate = sr; updateIncrement(); }
    void setFrequency(double freqHz) { freq = freqHz; updateIncrement(); }

    // Returns one sample in [-1, 1].
    double process() {
        double t = phase;
        double value = 2.0 * t - 1.0;          // naive saw
        value -= polyBlep(t);                   // band-limit the discontinuity
        phase += increment;
        if (phase >= 1.0) phase -= 1.0;
        return value;
    }

    void reset() { phase = 0.0; }

private:
    double polyBlep(double t) const {
        double dt = increment;
        if (t < dt) {                            // 0 <= t < dt
            t /= dt;
            return t + t - t * t - 1.0;
        }
        if (t > 1.0 - dt) {                      // 1-dt < t <= 1
            t = (t - 1.0) / dt;
            return t * t + t + t + 1.0;
        }
        return 0.0;
    }
    void updateIncrement() { increment = freq / sampleRate; }

    double sampleRate = 48000.0;
    double freq = 440.0;
    double phase = 0.0;
    double increment = 440.0 / 48000.0;
};

} // namespace absynth
