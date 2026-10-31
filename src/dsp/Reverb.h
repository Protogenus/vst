// Reverb.h — dark Freeverb-style algorithmic reverb. Fixed buffers, no allocation.
#pragma once
#include <cmath>
#include "DspCommon.h"

namespace absynth {

class Comb {
public:
    static constexpr int MaxLen = 4096;
    void setDelaySamples(double d) {
        delayLen = (int)d;
        if (delayLen >= MaxLen) delayLen = MaxLen - 1;
        if (delayLen < 1) delayLen = 1;
    }
    void setFeedback(double fb) { feedback = fb; }
    void setDamp(double d) { damp = d < 0 ? 0 : (d > 1 ? 1 : d); }
    void clear() {
        for (int i = 0; i < MaxLen; ++i) buf[i] = 0.0;
        idx = 0; filterStore = 0.0;
    }
    double process(double x) {
        double out = buf[idx];
        filterStore = out * (1.0 - damp) + filterStore * damp;
        buf[idx] = x + filterStore * feedback;
        if (++idx >= delayLen) idx = 0;
        return out;
    }
private:
    double buf[MaxLen] = {0};
    int idx = 0, delayLen = 100;
    double feedback = 0.8, damp = 0.5, filterStore = 0.0;
};

class Allpass {
public:
    static constexpr int MaxLen = 4096;
    void setDelaySamples(double d) {
        delayLen = (int)d;
        if (delayLen >= MaxLen) delayLen = MaxLen - 1;
        if (delayLen < 1) delayLen = 1;
    }
    void setFeedback(double fb) { feedback = fb; }
    void clear() {
        for (int i = 0; i < MaxLen; ++i) buf[i] = 0.0;
        idx = 0;
    }
    double process(double x) {
        double bufout = buf[idx];
        double out = -x + bufout;
        buf[idx] = x + bufout * feedback;
        if (++idx >= delayLen) idx = 0;
        return out;
    }
private:
    double buf[MaxLen] = {0};
    int idx = 0, delayLen = 100;
    double feedback = 0.5;
};

class Reverb {
public:
    static constexpr int NumCombs = 8;
    static constexpr int NumAllpasses = 4;

    void setSampleRate(double sr) {
        sampleRate = sr;
        double scale = sr / 44100.0;
        // Classic Freeverb tunings, stereo-spread on odd combs / even allpasses.
        const int combT[NumCombs] = {1116,1188,1277,1356,1422,1491,1557,1617};
        const int apT[NumAllpasses] = {556,441,341,225};
        for (int i = 0; i < NumCombs; ++i)
            combs[i].setDelaySamples(combT[i] * scale + (i & 1) * 23 * scale);
        for (int i = 0; i < NumAllpasses; ++i)
            allpasses[i].setDelaySamples(apT[i] * scale + (i & 1) * 23 * scale);
        for (auto& a : allpasses) a.setFeedback(0.5);
        clear();
    }

    void setRoomSize(double r01) { roomSize = r01 < 0 ? 0 : (r01 > 1 ? 1 : r01); update(); }
    void setDamp(double d01)     { damp = d01 < 0 ? 0 : (d01 > 1 ? 1 : d01); update(); }
    void setWet(double w01)      { wet = w01 < 0 ? 0 : (w01 > 1 ? 1 : w01); }

    void clear() {
        for (auto& c : combs) c.clear();
        for (auto& a : allpasses) a.clear();
    }

    // Wet-only output; caller mixes with dry.
    void process(double inL, double inR, double& wetL, double& wetR) {
        double input = (inL + inR) * 0.5;
        double l = 0.0, r = 0.0;
        for (int i = 0; i < NumCombs; i += 2) l += combs[i].process(input);
        for (int i = 1; i < NumCombs; i += 2) r += combs[i].process(input);
        l = allpasses[0].process(l); l = allpasses[2].process(l);
        r = allpasses[1].process(r); r = allpasses[3].process(r);
        wetL = l * 0.25 * wet;
        wetR = r * 0.25 * wet;
    }

private:
    void update() {
        double fb = 0.28 + roomSize * 0.70; // up to ~0.98
        for (auto& c : combs) { c.setFeedback(fb); c.setDamp(damp); }
    }
    double sampleRate = 48000.0;
    double roomSize = 0.65, damp = 0.7, wet = 0.25;
    Comb combs[NumCombs];
    Allpass allpasses[NumAllpasses];
};

} // namespace absynth
