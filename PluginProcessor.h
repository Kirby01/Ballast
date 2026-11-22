#pragma once

#include <JuceHeader.h>

class BallastAudioProcessor final : public juce::AudioProcessor
{
public:
    BallastAudioProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Ballast"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    static constexpr auto amountID = "amount";
    static constexpr auto outputID = "output";
    static constexpr auto deltaID = "delta";
    juce::AudioProcessorValueTreeState parameters;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
    void updateTargets();

    std::atomic<float>* const amountParameter;
    std::atomic<float>* const outputParameter;
    std::atomic<float>* const deltaParameter;
    juce::SmoothedValue<float> wetGain, outputGain, dryGain;
    double a = 0.0;
    double b = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BallastAudioProcessor)
};
