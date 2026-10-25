#include "PluginEditor.h"

JotaTuneAudioProcessorEditor::JotaTuneAudioProcessorEditor (JotaTuneAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      robotAtt (p.apvts, "robot", robotKnob),
      pitchAtt (p.apvts, "pitch", pitchKnob)
{
    auto setup = [this] (juce::Slider& s, juce::Label& l, const juce::String& text)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 22);
        s.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff00e5ff));
        s.setColour (juce::Slider::thumbColourId, juce::Colours::white);
        addAndMakeVisible (s);

        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setFont (juce::FontOptions (16.0f, juce::Font::bold));
        l.setColour (juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible (l);
    };

    setup (robotKnob, robotLabel, "ROBOT");
    setup (pitchKnob, pitchLabel, "TONO");
    pitchKnob.setTextValueSuffix (" st");

    setSize (420, 320);
}

void JotaTuneAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12121a));
    g.setColour (juce::Colour (0xff00e5ff));
    g.setFont (juce::FontOptions (30.0f, juce::Font::bold));
    g.drawText ("JotaTune", getLocalBounds().removeFromTop (60), juce::Justification::centred);
}

void JotaTuneAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (50);
    auto left = area.removeFromLeft (area.getWidth() / 2);

    robotLabel.setBounds (left.removeFromTop (24));
    robotKnob.setBounds (left);
    pitchLabel.setBounds (area.removeFromTop (24));
    pitchKnob.setBounds (area);
}
