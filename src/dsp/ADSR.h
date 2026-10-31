// ADSR.h — click-free exponential ADSR. All segments computed per-sample;
// no allocation, no branching surprises on the audio thread.
#pragma once
#include <cmath>

namespace absynth {

class ADSR {
public:
    enum class Stage { Idle, Attack, Decay, Sustain, Release };

    void setSampleRate(double sr) { sampleRate = sr; recalc(); }

    // times in seconds; sustain 0..1
    void set(double attackSec, double decaySec, double sustainLevel, double releaseSec) {
        a = attackSec; d = decaySec; s = sustainLevel; r = releaseSec;
        recalc();
    }

    void noteOn()  { stage = Stage::Attack; }
    void noteOff() { if (stage != Stage::Idle) stage = Stage::Release; }
    bool isActive() const { return stage != Stage::Idle; }

    double process() {
        switch (stage) {
            case Stage::Idle: return 0.0;
            case Stage::Attack:
                level += attackInc;
                if (level >= 1.0) { level = 1.0; stage = Stage::Decay; }
                break;
            case Stage::Decay:
                level = s + (level - s) * decayMul;
                if (level <= s + 1e-4) { level = s; stage = Stage::Sustain; }
                break;
            case Stage::Sustain: level = s; break;
            case Stage::Release:
                level *= releaseMul;
                if (level < 1e-4) { level = 0.0; stage = Stage::Idle; }
                break;
        }
        return level;
    }

    void reset() { stage = Stage::Idle; level = 0.0; }

private:
    void recalc() {
        attackInc = (a > 1e-6) ? (1.0 / (a * sampleRate)) : 1.0;
        // exponential: multiplier per sample to traverse in d seconds
        decayMul   = (d > 1e-6) ? std::exp(-5.0 / (d * sampleRate)) : 0.0;
        releaseMul = (r > 1e-6) ? std::exp(-5.0 / (r * sampleRate)) : 0.0;
    }

    double sampleRate = 48000.0;
    double a = 0.008, d = 0.1, s = 1.0, r = 0.08;
    double attackInc = 0, decayMul = 0, releaseMul = 0;
    double level = 0.0;
    Stage stage = Stage::Idle;
};

} // namespace absynth
