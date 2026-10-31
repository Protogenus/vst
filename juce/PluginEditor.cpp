// PluginEditor.cpp
#include "PluginEditor.h"

AbsoluteReasonLookAndFeel::AbsoluteReasonLookAndFeel() {
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffe8a33d));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a2e35));
    setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffd8dce3));
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff1a1d22));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff2a2e35));
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a1d22));
    setColour(juce::ComboBox::textColourId, juce::Colour(0xffd8dce3));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2a2e35));
    setColour(juce::Label::textColourId, juce::Colour(0xff9aa0ab));
    setColour(juce::TextButton::buttonColourId, juce::Colour(0xff22262d));
    setColour(juce::TextButton::textColourOnId, juce::Colour(0xffe8a33d));
    setColour(juce::TextButton::textColourOffId, juce::Colour(0xffd8dce3));
}

void AbsoluteReasonLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y,
        int width, int height, float sliderPos,
        float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) {
    const float radius = juce::jmin(width, height) * 0.42f;
    const float cx = x + width * 0.5f, cy = y + height * 0.5f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // track
    g.setColour(findColour(juce::Slider::rotarySliderOutlineColourId));
    g.drawEllipse(cx - radius, cy - radius, radius * 2, radius * 2, 5.0f);
    // value arc
    juce::Path arc;
    arc.addCentredArc(cx, cy, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    g.setColour(findColour(juce::Slider::rotarySliderFillColourId));
    g.strokePath(arc, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved));
    // knob body
    g.setColour(juce::Colour(0xff22262d));
    g.fillEllipse(cx - radius * 0.72f, cy - radius * 0.72f, radius * 1.44f, radius * 1.44f);
    g.setColour(juce::Colour(0xff3a4048));
    g.drawEllipse(cx - radius * 0.72f, cy - radius * 0.72f, radius * 1.44f, radius * 1.44f, 1.5f);
    // pointer
    juce::Path p;
    const float pr = radius * 0.62f;
    p.addLineSegment(juce::Line<float>(cx, cy, cx + std::sin(angle) * pr,
                                       cy - std::cos(angle) * pr), 3.0f);
    g.setColour(juce::Colour(0xffe8a33d));
    g.strokePath(p, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                         juce::PathStrokeType::rounded));
    juce::ignoreUnused(slider);
}

AbsoluteReasonEditor::AbsoluteReasonEditor(AbsoluteReasonProcessor& p)
    : AudioProcessorEditor(&p), proc(p) {
    setLookAndFeel(&lookAndFeel);
    setSize(660, 440);

    addAndMakeVisible(presetBox);
    presetBox.setJustificationType(juce::Justification::centredLeft);
    refreshPresetBox();
    presetBox.onChange = [this] {
        proc.applyPreset(presetBox.getSelectedItemIndex());
    };
    addAndMakeVisible(prevPreset);
    addAndMakeVisible(nextPreset);
    prevPreset.onClick = [this] {
        proc.applyPreset((proc.getCurrentProgram() + 2) % 3);
        refreshPresetBox();
    };
    nextPreset.onClick = [this] {
        proc.applyPreset((proc.getCurrentProgram() + 1) % 3);
        refreshPresetBox();
    };

    // Layout: macro knob big on the left; parameter groups on the right.
    addKnob("forward", "FORWARD", 36, 120, 120);

    const int kx = 230, ky = 96, dx = 96, dy = 104;
    addKnob("detune",  "Detune",  kx,         ky);
    addKnob("cutoff",  "Cutoff",  kx + dx,    ky);
    addKnob("resonance","Reso",   kx + dx*2,  ky);
    addKnob("attack",  "Attack",  kx,         ky + dy);
    addKnob("release", "Release", kx + dx,    ky + dy);
    addKnob("spread",  "Spread",  kx + dx*2,  ky + dy);
    addKnob("chorusDepth","Chorus", kx,       ky + dy*2);
    addKnob("chorusRate","Ch Rate",kx + dx,   ky + dy*2);
    addKnob("level",   "Level",   kx + dx*2,  ky + dy*2);
    addKnob("tune",    "Tune",    66, 300, 64);

    addAndMakeVisible(slopeBox);
    slopeBox.addItemList(proc.apvts.getParameter("slope")->getAllValueStrings(), 1);
    slopeBox.setSelectedId(2, juce::dontSendNotification);
    slopeLabel.setText("Slope", juce::dontSendNotification);
    slopeLabel.attachToComponent(&slopeBox, true);
    addAndMakeVisible(slopeLabel);
    slopeAttach = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            proc.apvts, "slope", slopeBox);
}

AbsoluteReasonEditor::~AbsoluteReasonEditor() {
    setLookAndFeel(nullptr);
}

void AbsoluteReasonEditor::addKnob(const juce::String& paramId,
                                   const juce::String& labelText,
                                   int x, int y, int size) {
    Knob k;
    k.slider = std::make_unique<juce::Slider>();
    k.slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    k.slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 18);
    k.slider->setBounds(x, y, size, size + 34);
    addAndMakeVisible(*k.slider);
    k.label = std::make_unique<juce::Label>();
    k.label->setText(labelText, juce::dontSendNotification);
    k.label->setJustificationType(juce::Justification::centred);
    k.label->setBounds(x, y - 20, size, 18);
    addAndMakeVisible(*k.label);
    k.attach = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            proc.apvts, paramId, *k.slider);
    knobs.push_back(std::move(k));
}

void AbsoluteReasonEditor::refreshPresetBox() {
    presetBox.clear(juce::dontSendNotification);
    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem(proc.getProgramName(i), i + 1);
    presetBox.setSelectedId(proc.getCurrentProgram() + 1, juce::dontSendNotification);
}

void AbsoluteReasonEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff14161a));
    // header
    g.setColour(juce::Colour(0xff1e2229));
    g.fillRect(0, 0, getWidth(), 64);
    g.setColour(juce::Colour(0xffe8a33d));
    g.setFont(juce::Font(22.0f, juce::Font::bold));
    g.drawText("ABSOLUTE REASON", 20, 12, 300, 26, juce::Justification::left);
    g.setColour(juce::Colour(0xff9aa0ab));
    g.setFont(juce::Font(12.0f));
    g.drawText("growling saw instrument", 20, 36, 300, 16, juce::Justification::left);
    // group labels
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff5a616c));
    g.drawText("OSC", 230, 78, 90, 14, juce::Justification::left);
    g.drawText("FILTER", 326, 78, 90, 14, juce::Justification::left);
    g.drawText("AMP / SPACE / MASTER", 230, 278, 280, 14, juce::Justification::left);
    // macro caption
    g.setColour(juce::Colour(0xff9aa0ab));
    g.setFont(juce::Font(11.0f));
    g.drawText("the \"comes forward\" control:", 24, 268, 160, 14, juce::Justification::centred);
    g.drawText("filter + level macro", 24, 282, 160, 14, juce::Justification::centred);
}

void AbsoluteReasonEditor::resized() {
    presetBox.setBounds(340, 18, 220, 28);
    prevPreset.setBounds(566, 18, 28, 28);
    nextPreset.setBounds(598, 18, 28, 28);
    slopeBox.setBounds(520, 330, 100, 24);
}
