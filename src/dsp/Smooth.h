// Smooth.h — click-free one-pole parameter smoother. No allocation.
#pragma once
#include <cmath>

namespace absynth {

class Smoother {
public:
    void setSampleRate(double sr) { sampleRate = sr; updateCoeff(); }
    // time for the value to converge (~95%) in milliseconds
    void setTimeMs(double ms) { timeMs = ms; updateCoeff(); }
    void setTarget(double v) { target = v; }
    void snapTo(double v) { current = target = v; }
    double process() {
        current += (target - current) * coeff;
        return current;
    }
    double get() const { return current; }

private:
    void updateCoeff() {
        double t = (timeMs * 0.001) * sampleRate;
        coeff = (t > 1.0) ? (1.0 - std::exp(-3.0 / t)) : 1.0;
    }
    double sampleRate = 48000.0, timeMs = 20.0;
    double current = 0.0, target = 0.0, coeff = 1.0;
};

} // namespace absynth
