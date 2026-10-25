#include "PluginProcessor.h"
#include "PluginEditor.h"

JotaTuneAudioProcessor::JotaTuneAudioProcessor()
    : AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout JotaTuneAudioProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "robot", 1 }, "Robot",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.7f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "pitch", 1 }, "Tono",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f));
    return layout;
}

void JotaTuneAudioProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    frame.assign (ringSize, 0.0f);
    yin.assign (ringSize, 1.0f);
    ring.fill (0.0f);
    for (auto& s : shifters) { s.buf.fill (0.0f); s.w = 0; s.phase = 0.0f; }
    ringPos = hopCount = 0;
    corr = desired = shiftNow = 0.0f;
}

bool JotaTuneAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    auto out = l.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return out == l.getMainInputChannelSet();
}

float JotaTuneAudioProcessor::Shifter::process (float in, float ratio)
{
    buf[(size_t) w] = in;

    phase += (1.0f - ratio) / (float) winLen;
    phase -= std::floor (phase);
    float p2 = phase + 0.5f;
    p2 -= std::floor (p2);

    auto read = [this] (float p)
    {
        float pos = (float) w - p * (float) winLen;
        while (pos < 0.0f) pos += (float) ringSize;
        int i0 = (int) pos;
        float fr = pos - (float) i0;
        float a = buf[(size_t) (i0 & ringMask)];
        float b = buf[(size_t) ((i0 + 1) & ringMask)];
        return a + fr * (b - a);
    };

    const float pi = juce::MathConstants<float>::pi;
    float s1 = std::sin (pi * phase);
    float s2 = std::sin (pi * p2);
    float out = read (phase) * s1 * s1 + read (p2) * s2 * s2;

    w = (w + 1) & ringMask;
    return out;
}

void JotaTuneAudioProcessor::analyse (float robot)
{
    const int W = 1024;
    const int minLag = juce::jmax (2, (int) (sr / 1000.0));
    const int maxLag = juce::jmin ((int) (sr / 70.0), ringSize - W - 2);
    const int total = W + maxLag + 1;

    for (int k = 0; k < total; ++k)
        frame[(size_t) k] = ring[(size_t) ((ringPos - total + k + ringSize) & ringMask)];

    float energy = 0.0f;
    for (int k = 0; k < W; ++k) energy += frame[(size_t) k] * frame[(size_t) k];
    energy = std::sqrt (energy / (float) W);

    if (energy > 0.005f)
    {
        yin[0] = 1.0f;
        float running = 0.0f;
        for (int tau = 1; tau <= maxLag; ++tau)
        {
            float d = 0.0f;
            for (int j = 0; j < W; ++j)
            {
                float diff = frame[(size_t) j] - frame[(size_t) (j + tau)];
                d += diff * diff;
            }
            running += d;
            yin[(size_t) tau] = running > 0.0f ? d * (float) tau / running : 1.0f;
        }

        int found = -1;
        for (int tau = minLag; tau < maxLag; ++tau)
        {
            if (yin[(size_t) tau] < 0.15f)
            {
                while (tau + 1 < maxLag && yin[(size_t) (tau + 1)] < yin[(size_t) tau]) ++tau;
                found = tau;
                break;
            }
        }

        if (found > 0)
        {
            float tauF = (float) found;
            if (found > 1 && found + 1 <= maxLag)
            {
                float a = yin[(size_t) (found - 1)], b = yin[(size_t) found], c = yin[(size_t) (found + 1)];
                float den = a - 2.0f * b + c;
                if (std::abs (den) > 1.0e-9f) tauF += 0.5f * (a - c) / den;
            }
            float freq = (float) sr / tauF;
            float midi = 69.0f + 12.0f * std::log2 (freq / 440.0f);
            desired = std::round (midi) - midi;
        }
    }

    float t = 0.4f * (1.0f - robot) * (1.0f - robot) + 0.0005f;
    float alpha = 1.0f - std::exp (-(float) hop / ((float) sr * t));
    corr += alpha * (desired - corr);
}

void JotaTuneAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int n = buffer.getNumSamples();
    const int nch = juce::jmin (buffer.getNumChannels(), 2);
    const float robot = apvts.getRawParameterValue ("robot")->load();
    const float pitch = apvts.getRawParameterValue ("pitch")->load();

    for (int i = 0; i < n; ++i)
    {
        ring[(size_t) ringPos] = buffer.getSample (0, i);
        ringPos = (ringPos + 1) & ringMask;

        if (++hopCount >= hop)
        {
            hopCount = 0;
            analyse (robot);
        }

        shiftNow += 0.01f * ((corr + pitch) - shiftNow);
        const float ratio = std::pow (2.0f, shiftNow / 12.0f);

        for (int c = 0; c < nch; ++c)
            buffer.setSample (c, i, shifters[c].process (buffer.getSample (c, i), ratio));
    }

    for (int c = nch; c < buffer.getNumChannels(); ++c)
        buffer.clear (c, 0, n);
}

juce::AudioProcessorEditor* JotaTuneAudioProcessor::createEditor()
{
    return new JotaTuneAudioProcessorEditor (*this);
}

void JotaTuneAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void JotaTuneAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JotaTuneAudioProcessor();
}
