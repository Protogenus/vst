// SynthEngine.h — polyphonic engine: voices + chorus + forward macro + smoothed params.
// All realtime-safe: fixed voice array, no allocation in process().
#pragma once
#include "Voice.h"
#include "Chorus.h"
#include "Reverb.h"
#include "Pump.h"
#include "Smooth.h"
#include <cmath>

namespace absynth {

struct Params {
    double detuneCents   = 20.0;   // total spread between the 2 saws (±10c each)
    double cutoffHz      = 1000.0; // filter cutoff at Forward=1 (A/B vs loud section)
    double resonance     = 0.0;    // 0..1 mapped to Q 0.5..2
    bool   slope24       = true;   // 24 vs 12 dB/oct
    double attackMs      = 8.0;
    double releaseMs     = 90.0;
    double chorusDepth   = 0.35;   // 0..1
    double chorusRateHz  = 0.35;
    double chorusMix     = 0.22;
    double panSpread     = 0.85;   // stereo split of the detuned pair
    double forward       = 1.0;    // macro 0..1: cutoff + level
    double level         = 0.8;    // master
    double masterTuneCents = 0.0;
    double drive         = 0.75;   // waveshaper growl 0..1 (0 = clean bypass)
    // FX (optional — all off at 0)
    double verbMix       = 0.25;   // reverb send 0..1
    double verbSize      = 0.65;   // room size 0..1
    double verbDark      = 0.70;   // damping 0..1 (darkness)
    double pumpAmt       = 0.25;   // pump depth 0..1
    double pumpRateHz    = 2.2;    // pump rate
};

struct MidiEvent {
    bool isNoteOn;   // else note-off
    int note;        // 0..127
    int sampleOffset;
};

class SynthEngine {
public:
    static constexpr int MaxVoices = 8;
    static constexpr int MaxEventsPerBlock = 256;

    void prepare(double sampleRate) {
        sr = sampleRate;
        for (auto& v : voices) v.setSampleRate(sampleRate);
        chorus.setSampleRate(sampleRate);
        reverb.setSampleRate(sampleRate);
        pump.setSampleRate(sampleRate);
        cutoffSm.setSampleRate(sampleRate); cutoffSm.setTimeMs(15);
        levelSm.setSampleRate(sampleRate);  levelSm.setTimeMs(15);
        detuneSm.setSampleRate(sampleRate); detuneSm.setTimeMs(30);
        verbMixSm.setSampleRate(sampleRate); verbMixSm.setTimeMs(30);
        // snap smoothers to current params
        applyParamsToSmoothers(true);
    }

    void setParams(const Params& p) {
        params = p;
        chorus.setRate(p.chorusRateHz);
        chorus.setDepth(p.chorusDepth);
        chorus.setMix(p.chorusMix);
        reverb.setRoomSize(p.verbSize);
        reverb.setDamp(p.verbDark);
        reverb.setWet(1.0); // mix handled by verbMixSm
        pump.setAmount(p.pumpAmt);
        pump.setRate(p.pumpRateHz);
        for (auto& v : voices) v.setDrive(p.drive);
        applyParamsToSmoothers(false);
    }
    const Params& getParams() const { return params; }

    void noteOn(int note, int /*velocity*/) {
        double freq = 440.0 * std::pow(2.0, (note - 69) / 12.0)
                    * std::pow(2.0, params.masterTuneCents / 1200.0);
        Voice* v = findFreeVoice();
        v->noteOn(note, freq, detuneSm.get(),
                  params.attackMs * 0.001, params.releaseMs * 0.001);
        v->setFilter(cutoffSm.get(), qFromRes(params.resonance), params.slope24);
    }

    void noteOff(int note) {
        for (auto& v : voices)
            if (v.isNote(note)) v.noteOff();
    }

    void allNotesOff() {
        for (auto& v : voices) v.kill();
        chorus.clear();
    }

    // events: array of MidiEvent, count <= MaxEventsPerBlock. Sorted by sampleOffset.
    void process(float* outL, float* outR, int numSamples,
                 const MidiEvent* events, int numEvents) {
        int evIdx = 0;
        int filterUpdateCounter = 0;
        for (int i = 0; i < numSamples; ++i) {
            while (evIdx < numEvents && events[evIdx].sampleOffset == i) {
                const auto& e = events[evIdx++];
                if (e.isNoteOn) noteOn(e.note, 100);
                else noteOff(e.note);
            }
            double cutoff = cutoffSm.process();
            double level  = levelSm.process();
            double detune = detuneSm.process();
            if ((filterUpdateCounter++ & 31) == 0) {
                double q = qFromRes(params.resonance);
                for (auto& v : voices) v.setFilter(cutoff, q, params.slope24);
            }
            (void)detune; // detune applies at noteOn (per-voice); smoothed value read there

            double mL = 0.0, mR = 0.0;
            for (auto& v : voices) {
                if (!v.isActive()) continue;
                double vl, vr;
                v.render(vl, vr, params.panSpread);
                mL += vl; mR += vr;
            }
            // gentle voice-sum normalization
            mL *= 0.5; mR *= 0.5;
            double oL, oR;
            chorus.process(mL, mR, oL, oR);
            // pump (rhythmic bus breathing), then dark reverb send
            pump.process(oL, oR);
            double wetL, wetR;
            reverb.process(oL, oR, wetL, wetR);
            double vmix = verbMixSm.process();
            oL += wetL * vmix * 2.0;
            oR += wetR * vmix * 2.0;
            outL[i] = (float)(oL * level);
            outR[i] = (float)(oR * level);
        }
    }

private:
    static double qFromRes(double r01) { return 0.5 + r01 * 1.5; }

    // Forward macro: cutoff scales down toward 1/4 and level toward 1/2 as macro -> 0
    void applyParamsToSmoothers(bool snap) {
        double effCutoff = params.cutoffHz * (0.25 + 0.75 * params.forward);
        double effLevel  = params.level * (0.5 + 0.5 * params.forward);
        if (snap) {
            cutoffSm.snapTo(effCutoff); levelSm.snapTo(effLevel);
            detuneSm.snapTo(params.detuneCents);
            verbMixSm.snapTo(params.verbMix);
            pump.snap();
        } else {
            cutoffSm.setTarget(effCutoff); levelSm.setTarget(effLevel);
            detuneSm.setTarget(params.detuneCents);
            verbMixSm.setTarget(params.verbMix);
        }
    }

    Voice* findFreeVoice() {
        for (auto& v : voices)
            if (!v.isActive()) return &v;
        // steal oldest
        Voice* oldest = &voices[0];
        for (auto& v : voices)
            if (v.getAge() > oldest->getAge()) oldest = &v;
        return oldest;
    }

    double sr = 48000.0;
    Params params;
    Voice voices[MaxVoices];
    Chorus chorus;
    Reverb reverb;
    Pump pump;
    Smoother cutoffSm, levelSm, detuneSm, verbMixSm;
};

} // namespace absynth
