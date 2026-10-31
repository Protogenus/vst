// PluginProcessor.cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

AbsoluteReasonProcessor::AbsoluteReasonProcessor()
    : AudioProcessor(BusesProperties()
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout()) {
    buildFactoryPresets();
    applyPreset(0);
}

AbsoluteReasonProcessor::~AbsoluteReasonProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout
AbsoluteReasonProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    auto pct = [](const juce::String& id, const juce::String& name, float def) {
        return std::make_unique<juce::AudioParameterFloat>(
            id, name, juce::NormalisableRange<float>(0.0f, 1.0f), def);
    };
    // The performance macro — the "comes forward" control from the track.
    layout.add(pct("forward", "Forward", 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "detune", "Detune", juce::NormalisableRange<float>(0.0f, 50.0f), 20.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "cutoff", "Cutoff",
        juce::NormalisableRange<float>(80.0f, 8000.0f, 0.0f, 0.35f), 1000.0f));
    layout.add(pct("resonance", "Resonance", 0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "slope", "Slope", juce::StringArray{"12 dB", "24 dB"}, 1));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "attack", "Attack",
        juce::NormalisableRange<float>(1.0f, 200.0f, 0.0f, 0.4f), 8.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "release", "Release",
        juce::NormalisableRange<float>(10.0f, 1000.0f, 0.0f, 0.4f), 90.0f));
    layout.add(pct("spread", "Stereo Spread", 0.85f));
    layout.add(pct("chorusDepth", "Chorus Depth", 0.35f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "chorusRate", "Chorus Rate",
        juce::NormalisableRange<float>(0.05f, 2.0f, 0.0f, 0.5f), 0.35f));
    layout.add(pct("level", "Level", 0.8f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "tune", "Master Tune",
        juce::NormalisableRange<float>(-50.0f, 50.0f), 0.0f));
    return layout;
}

void AbsoluteReasonProcessor::buildFactoryPresets() {
    factoryPresets = {
        {"Absolute Reason Growl", {{"forward",1.0f},{"detune",20.0f},{"cutoff",1000.0f},
            {"resonance",0.0f},{"slope",1.0f},{"attack",8.0f},{"release",90.0f},
            {"spread",0.85f},{"chorusDepth",0.35f},{"chorusRate",0.35f},
            {"level",0.8f},{"tune",0.0f}}},
        {"Background Dub", {{"forward",0.25f},{"detune",20.0f},{"cutoff",1000.0f},
            {"resonance",0.0f},{"slope",1.0f},{"attack",8.0f},{"release",90.0f},
            {"spread",0.85f},{"chorusDepth",0.35f},{"chorusRate",0.35f},
            {"level",0.8f},{"tune",0.0f}}},
        {"Bright Stab", {{"forward",1.0f},{"detune",28.0f},{"cutoff",2400.0f},
            {"resonance",0.15f},{"slope",1.0f},{"attack",4.0f},{"release",140.0f},
            {"spread",0.9f},{"chorusDepth",0.5f},{"chorusRate",0.5f},
            {"level",0.8f},{"tune",0.0f}}},
    };
}

void AbsoluteReasonProcessor::applyPreset(int index) {
    if (index < 0 || index >= (int)factoryPresets.size()) return;
    currentPreset = index;
    for (auto& kv : factoryPresets[(size_t)index].values)
        if (auto* p = apvts.getParameter(kv.first))
            p->setValueNotifyingHost(p->convertTo0to1(kv.second));
    dspParamsDirty = true;
}

void AbsoluteReasonProcessor::setCurrentProgram(int index) { applyPreset(index); }
const juce::String AbsoluteReasonProcessor::getProgramName(int index) {
    if (index >= 0 && index < (int)factoryPresets.size())
        return factoryPresets[(size_t)index].name;
    return {};
}

void AbsoluteReasonProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    engine.prepare(sampleRate);
    monoTemp.assign((size_t)samplesPerBlock, 0.0f);
    dspParamsDirty = true;
}

void AbsoluteReasonProcessor::releaseResources() { engine.allNotesOff(); }

void AbsoluteReasonProcessor::syncEngineParams() {
    absynth::Params p;
    auto getF = [&](const char* id) {
        return apvts.getRawParameterValue(id)->load();
    };
    p.forward    = getF("forward");
    p.detuneCents= getF("detune");
    p.cutoffHz   = getF("cutoff");
    p.resonance  = getF("resonance");
    p.slope24    = apvts.getRawParameterValue("slope")->load() > 0.5f;
    p.attackMs   = getF("attack");
    p.releaseMs  = getF("release");
    p.panSpread  = getF("spread");
    p.chorusDepth= getF("chorusDepth");
    p.chorusRateHz = getF("chorusRate");
    p.chorusMix  = 0.22;
    p.level      = getF("level");
    p.masterTuneCents = getF("tune");
    engine.setParams(p);
}

void AbsoluteReasonProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    // Merge on-screen keyboard MIDI (standalone) into the block.
    keyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true);

    if (dspParamsDirty) { syncEngineParams(); dspParamsDirty = false; }
    // Cheap per-block check: re-read the macro (most-performed param); full
    // sync happens via listener-free polling of a version counter is overkill —
    // instead poll every block; setParams is cheap (just targets).
    syncEngineParams();

    absynth::MidiEvent events[absynth::SynthEngine::MaxEventsPerBlock];
    int numEvents = 0;
    for (const auto metadata : midiMessages) {
        if (numEvents >= absynth::SynthEngine::MaxEventsPerBlock) break;
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            events[numEvents++] = {true, msg.getNoteNumber(), metadata.samplePosition};
        } else if (msg.isNoteOff()) {
            events[numEvents++] = {false, msg.getNoteNumber(), metadata.samplePosition};
        }
    }
    midiMessages.clear();

    auto* L = buffer.getWritePointer(0);
    auto* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
    if (R == nullptr) {
        // mono fallback (preallocated in prepareToPlay — never allocates here)
        engine.process(L, monoTemp.data(), numSamples, events, numEvents);
        for (int i = 0; i < numSamples; ++i) L[i] = (L[i] + monoTemp[i]) * 0.5f;
    } else {
        engine.process(L, R, numSamples, events, numEvents);
    }
}

void AbsoluteReasonProcessor::getStateInformation(juce::MemoryBlock& destData) {
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void AbsoluteReasonProcessor::setStateInformation(const void* data, int sizeInBytes) {
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
    dspParamsDirty = true;
}

juce::AudioProcessorEditor* AbsoluteReasonProcessor::createEditor() {
    return new AbsoluteReasonEditor(*this);
}

// Required JUCE entry point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new AbsoluteReasonProcessor();
}
