# Building "Absolute Reason" VST3 on Windows

You need a C++ compiler — the **Visual Studio Build Tools** (free, much smaller
than the full Visual Studio IDE), plus **CMake**.

## Install (one time)

In PowerShell or cmd:

```
winget install Microsoft.VisualStudio.2022.BuildTools
```

When the installer opens, tick **"Desktop development with C++"** and install.
(If you'd rather do it silently: `winget install Microsoft.VisualStudio.2022.BuildTools --override "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"`)

Then: `winget install Kitware.CMake`

## Steps

1. Get the source — either clone the repo or use the zip:
   - `git clone https://github.com/Protogenus/vst.git` (repo already contains JUCE 8.0.6 — no download needed), or
   - extract `vst3-absolute-reason.zip` — same contents.

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
