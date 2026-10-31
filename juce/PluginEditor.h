// PluginEditor.h — polished dark UI for Absolute Reason.
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

class AbsoluteReasonLookAndFeel : public juce::LookAndFeel_V4 {
public:
    AbsoluteReasonLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;
};

class AbsoluteReasonEditor : public juce::AudioProcessorEditor {
public:
    explicit AbsoluteReasonEditor(AbsoluteReasonProcessor&);
    ~AbsoluteReasonEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AbsoluteReasonProcessor& proc;
    AbsoluteReasonLookAndFeel lookAndFeel;

    juce::ComboBox presetBox;
    juce::TextButton prevPreset{"<"}, nextPreset{">"};
    juce::MidiKeyboardComponent keyboard;

    struct Knob {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };
    std::vector<Knob> knobs;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> slopeAttach;
    juce::ComboBox slopeBox;
    juce::Label slopeLabel;

    void addKnob(const juce::String& paramId, const juce::String& labelText,
                 int x, int y, int size = 72);
    void refreshPresetBox();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AbsoluteReasonEditor)
};
