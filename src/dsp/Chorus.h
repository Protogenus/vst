// Chorus.h — simple stereo chorus, fixed buffers (no audio-thread allocation).
#pragma once
#include <cmath>
#include "DspCommon.h"

namespace absynth {

class Chorus {
public:
    static constexpr int MaxDelay = 2048; // ~42ms at 48k

    void setSampleRate(double sr) {
        sampleRate = sr;
        lfoPhase = 0.0;
        updateLfoInc();
        clear();
    }
    void setRate(double hz) { rate = hz; updateLfoInc(); }
    void setDepth(double d01) { depth = d01 < 0 ? 0 : (d01 > 1 ? 1 : d01); } // 0..1
    void setMix(double m01) { mix = m01 < 0 ? 0 : (m01 > 1 ? 1 : m01); }

    void clear() {
        for (int i = 0; i < MaxDelay; ++i) bufL[i] = bufR[i] = 0.0;
        writePos = 0;
    }

    // Stereo in/out.
    void process(double inL, double inR, double& outL, double& outR) {
        bufL[writePos] = inL;
        bufR[writePos] = inR;

        // LFOs in antiphase for L/R
        double lfoL = std::sin(lfoPhase);
        double lfoR = std::sin(lfoPhase + kPi);
        lfoPhase += lfoInc;
        if (lfoPhase >= 2.0 * kPi) lfoPhase -= 2.0 * kPi;

        double baseMs = 11.0;
        double modMs = 5.0 * depth;
        double dL = (baseMs + modMs * lfoL) * 0.001 * sampleRate;
        double dR = (baseMs + modMs * lfoR) * 0.001 * sampleRate;

        double wetL = readDelay(bufL, dL);
        double wetR = readDelay(bufR, dR);

        outL = inL * (1.0 - mix) + wetL * mix;
        outR = inR * (1.0 - mix) + wetR * mix;

        if (++writePos >= MaxDelay) writePos = 0;
    }

private:
    double readDelay(const double* buf, double delaySamples) const {
        double rp = (double)writePos - delaySamples;
        while (rp < 0) rp += MaxDelay;
        int i0 = (int)rp;
        int i1 = (i0 + 1) % MaxDelay;
        double frac = rp - i0;
        return buf[i0] * (1.0 - frac) + buf[i1] * frac;
    }
    void updateLfoInc() { lfoInc = 2.0 * kPi * rate / sampleRate; }

    double sampleRate = 48000.0;
    double rate = 0.3, depth = 0.3, mix = 0.25;
    double lfoPhase = 0.0, lfoInc = 0.0;
    double bufL[MaxDelay] = {0}, bufR[MaxDelay] = {0};
    int writePos = 0;
};

} // namespace absynth
