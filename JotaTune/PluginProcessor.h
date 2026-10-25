#pragma once
#include <JuceHeader.h>

class JotaTuneAudioProcessor : public juce::AudioProcessor
{
public:
    JotaTuneAudioProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "JotaTune"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void analyse (float robot);

    static constexpr int ringSize = 4096;
    static constexpr int ringMask = ringSize - 1;
    static constexpr int hop = 256;
    static constexpr int winLen = 2048;

    std::array<float, ringSize> ring {};
    int ringPos = 0, hopCount = 0;
    std::vector<float> frame, yin;
    double sr = 44100.0;

    float corr = 0.0f;
    float desired = 0.0f;
    float shiftNow = 0.0f;

    struct Shifter
    {
        std::array<float, ringSize> buf {};
        int w = 0;
        float phase = 0.0f;
        float process (float in, float ratio);
    };
    Shifter shifters[2];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JotaTuneAudioProcessor)
};
