# Absolute Reason — VST3 Instrument

A growling detuned-saw synthesizer, reverse-engineered from the synth in
Stereo Dancer's "Absolute Reason" (Weaked Records, 1998).

## What it is

- **2 detuned sawtooth oscillators** (polyBLEP, ±10 cents default) — the growl
  is the detune beating, not distortion.
- **24 dB/oct lowpass filter**, no resonance by default.
- **FORWARD macro** — the big knob. This is the "comes forward" move from the
  track: it opens the filter and raises the level together, exactly like the
  synth does at ~0:15 and ~0:30 in the original.
- **Stereo unison spread + light chorus** — wide, wider with frequency.
- 8-voice polyphonic, 12 automatable parameters, 3 factory presets.
- Default preset **"Absolute Reason Growl"** is the loud forward sound.

## Build (Windows)

See **BUILD_WINDOWS.md** — Visual Studio 2022 + CMake, two commands.
JUCE 8.0.6 is included under `third_party/`; nothing to download.

## A/B validation

`ab/target_loud_31-36s.wav` is the original's loud section;
`ab/render_loud_preset.wav` is this synth's default preset playing the
transcribed riff. Smoothed harmonic profiles match within 2.78 dB RMSE
across the fundamental–5th harmonic. Full report:
`../synth-absolute-reason/report-phase1.md` (§8–9).

## Layout

- `src/dsp/` — dependency-free C++17 DSP core (shared by tests and plugin)
- `src/test/` — headless renderers used for A/B tuning
- `juce/` — VST3 wrapper (processor, editor, CMake)
- `ab/` — A/B audio and comparison scripts
- `third_party/JUCE/` — JUCE 8.0.6 (needed to build)
