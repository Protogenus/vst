# Building "Absolute Reason" VST3 on Windows

You need: **Visual Studio 2022** (free Community edition) with "Desktop development
with C++", and **CMake** (install from cmake.org, or `winget install Kitware.CMake`).

## Steps

1. Extract the zip so the folder `vst3-absolute-reason` is on your machine.
   It already contains JUCE 8.0.6 under `third_party/JUCE` — no download needed.

2. Open "x64 Native Tools Command Prompt for VS 2022" and run:

```
cd vst3-absolute-reason
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

3. The finished plugin is at:

```
build\AbsoluteReason_artefacts\Release\VST3\Absolute Reason.vst3
```

4. Copy the whole `Absolute Reason.vst3` folder to your VST3 folder
   (usually `C:\Program Files\Common Files\VST3\`), then rescan plugins
   in your DAW.

## Notes

- The default preset on load is **"Absolute Reason Growl"** — the loud forward
  sound from the track. The big **FORWARD** knob pulls it back to the dark
  background state, like the track does at 0:15 and 0:30.
- Two more factory presets: "Background Dub" and "Bright Stab".
- Your DAW handles saving/loading your own presets (the plugin reports its
  state to the host).
- If CMake can't find Visual Studio, run `cmake -B build` from inside the
  "x64 Native Tools" prompt (not plain cmd/PowerShell).
