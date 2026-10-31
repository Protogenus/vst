// PluginProcessor.h — "Absolute Reason" growling-saw VST3 instrument.
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "../src/dsp/SynthEngine.h"

class AbsoluteReasonProcessor : public juce::AudioProcessor {
public:
    AbsoluteReasonProcessor();
    ~AbsoluteReasonProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Absolute Reason"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override { return (int)factoryPresets.size(); }
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState; // feeds the on-screen keyboard

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void applyPreset(int index); // factory preset -> apvts

private:
    absynth::SynthEngine engine;
    absynth::Params cachedDspParams;
    bool dspParamsDirty = true;

    struct FactoryPreset { juce::String name; std::map<juce::String, float> values; };
    std::vector<FactoryPreset> factoryPresets;
    int currentPreset = 0;
    std::vector<float> monoTemp; // preallocated scratch for mono hosts
    void buildFactoryPresets();

    void syncEngineParams(); // apvts -> engine (called on audio thread, lock-free)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AbsoluteReasonProcessor)
};
