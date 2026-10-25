#pragma once
#include "PluginProcessor.h"

class JotaTuneAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit JotaTuneAudioProcessorEditor (JotaTuneAudioProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JotaTuneAudioProcessor& proc;
    juce::Slider robotKnob, pitchKnob;
    juce::Label robotLabel, pitchLabel;
    juce::AudioProcessorValueTreeState::SliderAttachment robotAtt, pitchAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JotaTuneAudioProcessorEditor)
};
