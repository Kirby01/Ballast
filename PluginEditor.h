#pragma once

#include "PluginProcessor.h"

class BallastAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit BallastAudioProcessorEditor(BallastAudioProcessor&);
    ~BallastAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class RackLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                              float position, float startAngle, float endAngle,
                              juce::Slider&) override;
        void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                              bool highlighted, bool down) override;
    };

    RackLookAndFeel rackLook;
    juce::Slider amountKnob, outputKnob;
    juce::ToggleButton deltaButton{ "DELTA" };
    juce::AudioProcessorValueTreeState::SliderAttachment amountAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment outputAttachment;
    juce::AudioProcessorValueTreeState::ButtonAttachment deltaAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BallastAudioProcessorEditor)
};
