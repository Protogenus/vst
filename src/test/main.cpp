// main.cpp — headless renderer: plays the default preset, writes WAV for A/B.
#include "../dsp/SynthEngine.h"
#include <cstdio>
#include <cstdint>
#include <vector>
#include <cstring>

using namespace absynth;

static void writeWavFloat(const char* path, const std::vector<float>& L,
                          const std::vector<float>& R, int sr) {
    FILE* f = std::fopen(path, "wb");
    uint32_t n = (uint32_t)L.size();
    uint32_t dataBytes = n * 2 * sizeof(float);
    uint32_t chunkSize = 36 + dataBytes;
    std::fwrite("RIFF", 1, 4, f);
    std::fwrite(&chunkSize, 4, 1, f);
    std::fwrite("WAVE", 1, 4, f);
    std::fwrite("fmt ", 1, 4, f);
    uint32_t fmtLen = 16; std::fwrite(&fmtLen, 4, 1, f);
    uint16_t audioFmt = 3; std::fwrite(&audioFmt, 2, 1, f); // float
    uint16_t ch = 2; std::fwrite(&ch, 2, 1, f);
    uint32_t sru = (uint32_t)sr; std::fwrite(&sru, 4, 1, f);
    uint32_t byteRate = sru * 2 * sizeof(float); std::fwrite(&byteRate, 4, 1, f);
    uint16_t blockAlign = 2 * sizeof(float); std::fwrite(&blockAlign, 2, 1, f);
    uint16_t bits = 32; std::fwrite(&bits, 2, 1, f);
    std::fwrite("data", 1, 4, f);
    std::fwrite(&dataBytes, 4, 1, f);
    for (uint32_t i = 0; i < n; ++i) {
        std::fwrite(&L[i], 4, 1, f);
        std::fwrite(&R[i], 4, 1, f);
    }
    std::fclose(f);
}

int main() {
    const int SR = 48000;
    SynthEngine eng;
    eng.prepare(SR);

    Params p; // default preset: "Absolute Reason Growl"
    p.detuneCents = 20.0;
    p.cutoffHz = 2500.0;      // starting value — tuned by A/B below
    p.resonance = 0.0;
    p.slope24 = true;
    p.attackMs = 8.0;
    p.releaseMs = 90.0;
    p.chorusDepth = 0.35;
    p.chorusRateHz = 0.35;
    p.chorusMix = 0.22;
    p.panSpread = 0.85;
    p.forward = 1.0;
    p.level = 0.8;
    eng.setParams(p);

    struct Note { int midi; double beats; };
    // riff: F#3 pedal with D#4 stabs (from Phase 3 transcription)
    std::vector<Note> riff = {
        {54, 3.0}, {63, 0.5}, {54, 2.0}, {63, 0.5}, {54, 3.0},
    };

    std::vector<float> L, R;
    L.reserve(SR * 12); R.reserve(SR * 12);
    const int blockSize = 512;
    std::vector<float> bL(blockSize), bR(blockSize);

    auto renderNote = [&](int midi, double seconds) {
        int total = (int)(seconds * SR);
        MidiEvent on{true, midi, 0};
        int done = 0;
        // note-on in first block
        while (done < total) {
            int n = blockSize < (total - done) ? blockSize : (total - done);
            int nev = 0; MidiEvent ev[2];
            if (done == 0) { ev[0] = on; nev = 1; }
            eng.process(bL.data(), bR.data(), n, ev, nev);
            L.insert(L.end(), bL.begin(), bL.begin() + n);
            R.insert(R.end(), bR.begin(), bR.begin() + n);
            done += n;
        }
        MidiEvent off{false, midi, 0};
        int relTotal = (int)(0.4 * SR); done = 0;
        while (done < relTotal) {
            int n = blockSize < (relTotal - done) ? blockSize : (relTotal - done);
            int nev = 0; MidiEvent ev[2];
            if (done == 0) { ev[0] = off; nev = 1; }
            eng.process(bL.data(), bR.data(), n, ev, nev);
            L.insert(L.end(), bL.begin(), bL.begin() + n);
            R.insert(R.end(), bR.begin(), bR.begin() + n);
            done += n;
        }
    };

    for (auto& nt : riff) renderNote(nt.midi, nt.beats);

    writeWavFloat("/home/hatch/workspace/vst3-absolute-reason/ab/render_v1.wav", L, R, SR);
    std::printf("rendered %zu samples -> ab/render_v1.wav\n", L.size());
    return 0;
}
