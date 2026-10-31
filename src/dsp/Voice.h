// Voice.h — one polyphonic voice: 2 detuned saws, stereo, per-channel lowpass, ADSR.
#pragma once
#include "SawOsc.h"
#include "LowpassFilter.h"
#include "ADSR.h"

namespace absynth {

class Voice {
public:
    void setSampleRate(double sr) {
        oscA.setSampleRate(sr); oscB.setSampleRate(sr);
        filtL.setSampleRate(sr); filtR.setSampleRate(sr);
        amp.setSampleRate(sr);
    }

    // detuneCentsTotal: total spread between the two oscs (each gets half, opposite signs)
    void noteOn(int midiNote, double freqHz, double detuneCentsTotal,
                double attackSec, double releaseSec) {
        note = midiNote;
        double ratio = std::pow(2.0, (detuneCentsTotal * 0.5) / 1200.0);
        oscA.setFrequency(freqHz / ratio);
        oscB.setFrequency(freqHz * ratio);
        oscA.reset(); oscB.reset();
        filtL.reset(); filtR.reset();
        amp.set(attackSec, 0.05, 1.0, releaseSec);
        amp.noteOn();
        active = true;
        age = 0;
    }

    void noteOff() { amp.noteOff(); }
    bool isActive() const { return active; }
    bool isNote(int m) const { return active && note == m; }
    void setAge(unsigned long a) { age = a; }
    unsigned long getAge() const { return age; }

    void setFilter(double cutoffHz, double q, bool poles24) {
        filtL.setCutoff(cutoffHz); filtR.setCutoff(cutoffHz);
        filtL.setQ(q); filtR.setQ(q);
        filtL.setPoles(poles24); filtR.setPoles(poles24);
    }

    // Renders one stereo sample. Pan: oscA leans left, oscB leans right
    // (this is what makes width grow with frequency — matches the measurement).
    void render(double& outL, double& outR, double panSpread) {
        double env = amp.process();
        if (!amp.isActive()) { active = false; outL = outR = 0.0; return; }
        double a = oscA.process();
        double b = oscB.process();
        // oscA leans left, oscB leans right; spread 0 = mono, 1 = hard split
        double l = a * (0.5 + 0.5 * panSpread) + b * (0.5 - 0.5 * panSpread);
        double r = b * (0.5 + 0.5 * panSpread) + a * (0.5 - 0.5 * panSpread);
        // drive: tanh waveshaper, the growl. drive=0 is true bypass.
        if (drive > 0.001) {
            double k = 1.0 + drive * 15.0;
            double norm = std::tanh(k);
            double dl = std::tanh(l * k) / norm;
            double dr = std::tanh(r * k) / norm;
            l += drive * (dl - l);
            r += drive * (dr - r);
        }
        outL = filtL.process(l) * env;
        outR = filtR.process(r) * env;
        ++age;
    }

    void setDrive(double d) { drive = d < 0 ? 0 : (d > 1 ? 1 : d); }

    void kill() { active = false; amp.reset(); }

private:
    SawOsc oscA, oscB;
    LowpassFilter filtL, filtR;
    ADSR amp;
    double drive = 0.0;
    bool active = false;
    int note = -1;
    unsigned long age = 0;
};

} // namespace absynth
