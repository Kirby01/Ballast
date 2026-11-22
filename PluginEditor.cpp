#include "PluginEditor.h"
#include <cmath>

namespace
{
const juce::Colour ink{ 0xff201d1a };

juce::Font rackFont(float height, bool serif = false, bool italic = false)
{
    const auto family = serif ? juce::Font::getDefaultSerifFontName()
                              : juce::Font::getDefaultSansSerifFontName();
    const auto style = italic ? juce::Font::bold | juce::Font::italic : juce::Font::bold;
   #if JUCE_MAJOR_VERSION >= 8
    return juce::Font(juce::FontOptions(family, height, style));
   #else
    return juce::Font(family, height, style);
   #endif
}

void configureKnob(juce::Slider& knob, const juce::String& title,
                   const juce::String& suffix, int textWidth, double resetValue)
{
    knob.setName(title);
    knob.setTitle(title);
    knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, 21);
    knob.setTextValueSuffix(suffix);
    knob.setDoubleClickReturnValue(true, resetValue);
    knob.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    knob.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff262320));
    knob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff59534d));
    knob.setColour(juce::Slider::textBoxHighlightColourId, juce::Colour(0xff665c52));
}
}

BallastAudioProcessorEditor::BallastAudioProcessorEditor(BallastAudioProcessor& processor)
    : AudioProcessorEditor(&processor),
      amountAttachment(processor.parameters, BallastAudioProcessor::amountID, amountKnob),
      outputAttachment(processor.parameters, BallastAudioProcessor::outputID, outputKnob),
      deltaAttachment(processor.parameters, BallastAudioProcessor::deltaID, deltaButton)
{
    setLookAndFeel(&rackLook);
    configureKnob(amountKnob, "Bass Amount", "%", 64, 66.0);
    configureKnob(outputKnob, "Output", " dB", 70, 0.0);
    deltaButton.setTitle("Delta Listen");
    addAndMakeVisible(amountKnob);
    addAndMakeVisible(outputKnob);
    addAndMakeVisible(deltaButton);
    setSize(720, 160);
    setOpaque(true);
}

BallastAudioProcessorEditor::~BallastAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void BallastAudioProcessorEditor::resized()
{
    amountKnob.setBounds(394, 7, 137, 146);
    outputKnob.setBounds(568, 24, 106, 85);
    deltaButton.setBounds(582, 118, 91, 27);
}

void BallastAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffeeeae5), 0.0f, 0.0f,
                                          juce::Colour(0xffaaa49e), 0.0f, 160.0f, false));
    g.fillAll();
    juce::Random grain(120);
    for (int y = 1; y < getHeight(); ++y)
    {
        const bool light = grain.nextBool();
        g.setColour((light ? juce::Colours::white : ink).withAlpha(grain.nextFloat() * 0.12f));
        g.drawHorizontalLine(y, 0.0f, static_cast<float>(getWidth()));
    }

    g.setColour(ink.withAlpha(0.65f));
    g.drawRect(getLocalBounds());
    for (const int x : { 29, 299, 548, 691 })
        g.drawVerticalLine(x, 10.0f, 150.0f);

    for (const int x : { 5, 698 })
        for (const int y : { 12, 138 })
        {
            g.setColour(juce::Colours::white.withAlpha(0.72f));
            g.fillRoundedRectangle(static_cast<float>(x), static_cast<float>(y + 1), 17.0f, 10.0f, 5.0f);
            g.setColour(ink);
            g.fillRoundedRectangle(static_cast<float>(x), static_cast<float>(y), 17.0f, 9.0f, 4.5f);
        }

    g.setColour(ink);
    g.setFont(rackFont(16.0f, true));
    g.drawText("HARMONIOUS", 43, 18, 240, 18, juce::Justification::centredLeft);
    g.drawText("RECORDS", 43, 36, 240, 18, juce::Justification::centredLeft);
    g.drawHorizontalLine(63, 43.0f, 281.0f);
    g.setFont(rackFont(51.0f, true, true));
    g.drawFittedText("BALLAST", 41, 68, 245, 65, juce::Justification::centredLeft, 1);
    g.setFont(rackFont(15.0f, true));
    g.drawText("BASS", 308, 53, 85, 19, juce::Justification::centredRight);
    g.drawText("AMOUNT", 308, 71, 85, 19, juce::Justification::centredRight);
    g.drawText("OUTPUT", 568, 7, 106, 18, juce::Justification::centred);
}

void BallastAudioProcessorEditor::RackLookAndFeel::drawRotarySlider(
    juce::Graphics& g, int x, int y, int width, int height, float position,
    float startAngle, float endAngle, juce::Slider&)
{
    const juce::Point<float> centre{ static_cast<float>(x) + 0.5f * width,
                                    static_cast<float>(y) + 0.5f * height };
    const float radius = 0.5f * static_cast<float>(juce::jmin(width, height)) - 10.0f;
    const juce::Rectangle<float> disc(centre.x - radius, centre.y - radius,
                                       2.0f * radius, 2.0f * radius);

    g.setColour(ink.withAlpha(0.8f));
    for (int tick = 0; tick <= 10; ++tick)
    {
        const float angle = juce::jmap(tick / 10.0f, startAngle, endAngle);
        const juce::Point<float> direction{ std::sin(angle), -std::cos(angle) };
        g.drawLine(juce::Line<float>(centre + direction * (radius + 3.0f),
                                     centre + direction * (radius + 7.0f)), 1.1f);
    }

    g.setColour(ink.withAlpha(0.35f));
    g.fillEllipse(disc.translated(0.0f, 3.0f));
    g.setColour(ink);
    g.fillEllipse(disc);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xfff7f4ef), disc.getTopLeft(),
                                          juce::Colour(0xff776f65), disc.getBottomRight(), false));
    g.fillEllipse(disc.reduced(1.3f));
    g.setColour(juce::Colours::white.withAlpha(0.17f));
    for (float ring = 3.0f; ring < radius - 2.0f; ring += 2.0f)
        g.drawEllipse(disc.reduced(ring), 0.45f);

    const float angle = juce::jmap(position, startAngle, endAngle);
    const juce::Point<float> direction{ std::sin(angle), -std::cos(angle) };
    g.setColour(ink);
    g.drawLine(juce::Line<float>(centre + direction * (radius * 0.51f),
                                 centre + direction * (radius - 4.0f)), 2.6f);
}

void BallastAudioProcessorEditor::RackLookAndFeel::drawToggleButton(
    juce::Graphics& g, juce::ToggleButton& button, bool highlighted, bool down)
{
    const auto lamp = juce::Rectangle<float>(2.0f, 2.0f, 23.0f, 23.0f);
    auto red = juce::Colour(button.getToggleState() ? 0xffef3326 : 0xff671510);
    if (highlighted || down)
        red = red.brighter(0.25f);

    g.setColour(ink);
    g.fillRoundedRectangle(lamp, 3.0f);
    g.setGradientFill(juce::ColourGradient(red.brighter(0.35f), lamp.getTopLeft(),
                                          red.darker(0.40f), lamp.getBottomRight(), false));
    g.fillRoundedRectangle(lamp.reduced(3.0f), 2.0f);
    g.setColour(ink);
    g.setFont(rackFont(15.0f, true));
    g.drawText(button.getButtonText(), 33, 0, button.getWidth() - 33, button.getHeight(),
               juce::Justification::centredLeft);
}
