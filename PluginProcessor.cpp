#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

BallastAudioProcessor::BallastAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "BallastState", makeLayout()),
      amountParameter(parameters.getRawParameterValue(amountID)),
      outputParameter(parameters.getRawParameterValue(outputID)),
      deltaParameter(parameters.getRawParameterValue(deltaID))
{
}

juce::AudioProcessorValueTreeState::ParameterLayout BallastAudioProcessor::makeLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ amountID, 1 }, "Bass Amount",
        juce::NormalisableRange<float>{ 0.0f, 100.0f, 1.0f }, 66.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ outputID, 1 }, "Output",
        juce::NormalisableRange<float>{ -24.0f, 24.0f, 0.1f }, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ deltaID, 1 }, "Delta", false));
    return layout;
}

void BallastAudioProcessor::prepareToPlay(double sampleRate, int)
{
    a = b = 0.0;
    wetGain.reset(sampleRate, 0.02);
    dryGain.reset(sampleRate, 0.02);
    outputGain.reset(sampleRate, 0.02);
    updateTargets();
    wetGain.setCurrentAndTargetValue(wetGain.getTargetValue());
    dryGain.setCurrentAndTargetValue(dryGain.getTargetValue());
    outputGain.setCurrentAndTargetValue(outputGain.getTargetValue());
}

bool BallastAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto channels = layouts.getMainOutputChannelSet();
    return channels == layouts.getMainInputChannelSet()
        && (channels == juce::AudioChannelSet::mono()
            || channels == juce::AudioChannelSet::stereo());
}

void BallastAudioProcessor::updateTargets()
{
    wetGain.setTargetValue(0.5f * amountParameter->load());
    outputGain.setTargetValue(juce::Decibels::decibelsToGain(outputParameter->load()));
    dryGain.setTargetValue(deltaParameter->load() >= 0.5f ? 0.0f : 1.0f);
}

void BallastAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    updateTargets();

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float l = std::isfinite(left[i]) ? left[i] : 0.0f;
        const float r = right != nullptr && std::isfinite(right[i]) ? right[i] : 0.0f;
        const double mid = right != nullptr ? 0.5 * (static_cast<double>(l) + r) : l;

        // Bound the detector on over-range input and retain the original recurrence.
        a = b + 0.001 * (juce::jlimit(-1.0, 1.0, mid) - a);
        b = a + 0.001 * (a - b) * 22.0 * (a + b) * 22.0;
        b = juce::jlimit(-1.0, 1.0, b);

        const float wet = static_cast<float>(b) * wetGain.getNextValue();
        const float dry = dryGain.getNextValue();
        const float trim = outputGain.getNextValue();

        left[i] = (dry * l + wet) * trim;
        if (right != nullptr)
            right[i] = (dry * r + wet) * trim;
    }
}

juce::AudioProcessorEditor* BallastAudioProcessor::createEditor()
{
    return new BallastAudioProcessorEditor(*this);
}

void BallastAudioProcessor::getStateInformation(juce::MemoryBlock& data)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary(*xml, data);
}

void BallastAudioProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BallastAudioProcessor();
}
