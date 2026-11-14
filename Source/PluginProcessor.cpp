#include "PluginProcessor.h"
#include "PluginEditor.h"

PepelSpaceAudioProcessor::PepelSpaceAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParameters()) {}

PepelSpaceAudioProcessor::~PepelSpaceAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout PepelSpaceAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto addF = [&p](const char* id, const char* name, float mn, float mx, float def, float step = 0.01f) {
        p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(id, 1), name, juce::NormalisableRange<float>(mn, mx, step), def));
    };
    auto addB = [&p](const char* id, const char* name, bool def) {
        p.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID(id, 1), name, def));
    };

    addF("input_gain", "Input (dB)", -24.0f, 24.0f, 0.0f);
    addF("output_gain", "Output (dB)", -24.0f, 24.0f, 0.0f);
    addF("dry_wet", "Mix", 0.0f, 1.0f, 1.0f);
    addB("bypass", "Bypass", false);

    addF("width", "Width %", 0.0f, 200.0f, 100.0f);
    addF("mono_bass_freq", "MB Freq", 20.0f, 300.0f, 120.0f);
    addF("mono_bass_amount", "MB Amt", 0.0f, 100.0f, 0.0f);
    addF("mid_gain", "Mid %", 0.0f, 200.0f, 100.0f);
    addF("side_gain", "Side %", 0.0f, 200.0f, 100.0f);

    addF("pan", "Pan L-R", -1.0f, 1.0f, 0.0f);
    addF("depth", "Depth F-B", 0.0f, 100.0f, 0.0f);
    addF("height", "Height U-D", -100.0f, 100.0f, 0.0f);

    addF("delay_l", "Delay L", 0.0f, 30.0f, 0.0f);
    addF("delay_r", "Delay R", 0.0f, 30.0f, 0.0f);
    addB("delay_on", "Delay On", false);

    addB("phase_invert_l", "Phase L", false);
    addB("phase_invert_r", "Phase R", false);
    addB("channel_swap", "Swap L/R", false);
    addB("mono_check", "Mono Check", false);

    return { p.begin(), p.end() };
}

void PepelSpaceAudioProcessor::prepareToPlay(double sr, int blockSize)
{
    currentSampleRate = sr;
    setLatencySamples(0);

    smoothedInGain.reset(sr, 0.02); smoothedOutGain.reset(sr, 0.02);
    smoothedDryWet.reset(sr, 0.02); smoothedWidth.reset(sr, 0.02);
    smoothedInGain.setCurrentAndTargetValue(1.0f); smoothedOutGain.setCurrentAndTargetValue(1.0f);
    smoothedDryWet.setCurrentAndTargetValue(1.0f); smoothedWidth.setCurrentAndTargetValue(1.0f);

    mbLP_L = mbLP_R = depthLP_L = depthLP_R = heightLP_L = heightLP_R = 0.0f;

    delayBufSize = (int)(sr * 0.05) + 2;
    delayBufL.setSize(1, delayBufSize); delayBufL.clear();
    delayBufR.setSize(1, delayBufSize); delayBufR.clear();
    delayWritePos = 0;
    dryBuffer.setSize(2, blockSize, false, true, true);
}

void PepelSpaceAudioProcessor::releaseResources() {}

void PepelSpaceAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    if (buffer.getNumChannels() < 2 || n == 0) return;

    auto* L = buffer.getWritePointer(0);
    auto* R = buffer.getWritePointer(1);
    auto get = [&](const char* id) { return apvts.getRawParameterValue(id)->load(); };

    if (get("bypass") > 0.5f) return;

    const float inDb = get("input_gain"), outDb = get("output_gain"), dw = get("dry_wet");
    const float widthK = get("width") * 0.01f, mbFreq = get("mono_bass_freq"), mbAmt = get("mono_bass_amount") * 0.01f;
    const float midK = get("mid_gain") * 0.01f, sideK = get("side_gain") * 0.01f;
    const float pan = get("pan"), depth = get("depth") * 0.01f, height = get("height") * 0.01f;
    const float dLyMs = get("delay_l"), dRyMs = get("delay_r");
    const bool dOn = get("delay_on") > 0.5f, phL = get("phase_invert_l") > 0.5f, phR = get("phase_invert_r") > 0.5f;
    const bool swap = get("channel_swap") > 0.5f, monoChk = get("mono_check") > 0.5f;

    smoothedInGain.setTargetValue(juce::Decibels::decibelsToGain(inDb));
    smoothedOutGain.setTargetValue(juce::Decibels::decibelsToGain(outDb));
    smoothedDryWet.setTargetValue(dw);
    smoothedWidth.setTargetValue(widthK);

    dryBuffer.setSize(2, n, false, false, true);
    dryBuffer.copyFrom(0, 0, L, n);
    dryBuffer.copyFrom(1, 0, R, n);

    const float mbCoeff = std::exp(-2.0f * juce::MathConstants<float>::pi * mbFreq / (float)currentSampleRate);
    const float depthCut = 20000.0f * std::pow(0.1f, depth);
    const float depthCoeff = std::exp(-2.0f * juce::MathConstants<float>::pi * depthCut / (float)currentSampleRate);
    const float depthGain = 1.0f - depth * 0.5f;
    const float heightLPc = std::exp(-2.0f * juce::MathConstants<float>::pi * 2000.0f / (float)currentSampleRate);
    
    const float panAngle = (pan + 1.0f) * 0.5f * juce::MathConstants<float>::halfPi;
    const float panL = std::cos(panAngle) * 1.41421356f, panR = std::sin(panAngle) * 1.41421356f;

    for (int i = 0; i < n; ++i)
    {
        float l = L[i] * smoothedInGain.getNextValue();
        float r = R[i] * smoothedInGain.getCurrentValue();

        depthLP_L = l * (1.0f - depthCoeff) + depthLP_L * depthCoeff;
        depthLP_R = r * (1.0f - depthCoeff) + depthLP_R * depthCoeff;
        l = depthLP_L * depthGain;
        r = depthLP_R * depthGain;

        heightLP_L = l * (1.0f - heightLPc) + heightLP_L * heightLPc;
        heightLP_R = r * (1.0f - heightLPc) + heightLP_R * heightLPc;
        l += (l - heightLP_L) * height * 0.8f;
        r += (r - heightLP_R) * height * 0.8f;

        mbLP_L = l * (1.0f - mbCoeff) + mbLP_L * mbCoeff;
        mbLP_R = r * (1.0f - mbCoeff) + mbLP_R * mbCoeff;
        const float lowMono = (mbLP_L + mbLP_R) * 0.5f;
        l = l - mbLP_L * mbAmt + lowMono * mbAmt;
        r = r - mbLP_R * mbAmt + lowMono * mbAmt;

        float mid = (l + r) * 0.5f, side = (l - r) * 0.5f;
        mid *= midK;
        side *= sideK * smoothedWidth.getCurrentValue();
        l = mid + side; r = mid - side;

        l *= panL; r *= panR;
        L[i] = l; R[i] = r;
    }
    smoothedInGain.skip(n - 1); smoothedWidth.skip(n - 1);

    if (dOn && delayBufSize > 2) {
        int dSL = juce::jlimit(1, delayBufSize - 1, (int)(dLyMs * 0.001f * currentSampleRate));
        int dSR = juce::jlimit(1, delayBufSize - 1, (int)(dRyMs * 0.001f * currentSampleRate));
        auto* bL = delayBufL.getWritePointer(0); auto* bR = delayBufR.getWritePointer(0);
        for (int i = 0; i < n; ++i) {
            int w = delayWritePos;
            float outL = bL[(w - dSL + delayBufSize) % delayBufSize];
            float outR = bR[(w - dSR + delayBufSize) % delayBufSize];
            bL[w] = L[i]; bR[w] = R[i];
            L[i] = outL; R[i] = outR;
            delayWritePos = (delayWritePos + 1) % delayBufSize;
        }
    }

    if (phL) for (int i = 0; i < n; ++i) L[i] = -L[i];
    if (phR) for (int i = 0; i < n; ++i) R[i] = -R[i];

    if (swap) {
        juce::AudioBuffer<float> tmp(1, n); tmp.copyFrom(0, 0, L, n);
        buffer.copyFrom(0, 0, R, n); buffer.copyFrom(1, 0, tmp.getReadPointer(0), n);
        L = buffer.getWritePointer(0); R = buffer.getWritePointer(1);
    }

    if (monoChk) {
        for (int i = 0; i < n; ++i) { float m = (L[i] + R[i]) * 0.5f; L[i] = m; R[i] = m; }
    }

    for (int ch = 0; ch < 2; ++ch) {
        auto* wet = buffer.getWritePointer(ch); auto* dry = dryBuffer.getReadPointer(ch);
        for (int i = 0; i < n; ++i) wet[i] = dry[i] * (1.0f - smoothedDryWet.getCurrentValue()) + wet[i] * smoothedDryWet.getCurrentValue();
    }
    smoothedDryWet.skip(n - 1);

    float g = smoothedOutGain.getCurrentValue(), target = smoothedOutGain.getTargetValue(), step = (target - g) / juce::jmax(1, n);
    for (int i = 0; i < n; ++i) { g += step; L[i] *= g; R[i] *= g; }
    smoothedOutGain.setCurrentAndTargetValue(target);

    float sumLR = 0, sumLL = 0, sumRR = 0, peakL = 0, peakR = 0;
    for (int i = 0; i < n; ++i) {
        sumLR += L[i] * R[i]; sumLL += L[i] * L[i]; sumRR += R[i] * R[i];
        peakL = juce::jmax(peakL, std::abs(L[i])); peakR = juce::jmax(peakR, std::abs(R[i]));
    }
    const float denom = std::sqrt(sumLL * sumRR);
    const float corr = denom > 1e-9f ? sumLR / denom : 1.0f;
    currentCorrelation.store(currentCorrelation.load() * 0.85f + corr * 0.15f);
    currentLevelL.store(peakL); currentLevelR.store(peakR);
}

juce::AudioProcessorEditor* PepelSpaceAudioProcessor::createEditor() { return new PepelSpaceAudioProcessorEditor(*this); }
void PepelSpaceAudioProcessor::getStateInformation(juce::MemoryBlock& dest) { if (auto s = apvts.copyState().createXml()) copyXmlToBinary(*s, dest); }
void PepelSpaceAudioProcessor::setStateInformation(const void* data, int size) { if (auto xml = getXmlFromBinary(data, size)) if (xml->hasTagName(apvts.state.getType())) apvts.replaceState(juce::ValueTree::fromXml(*xml)); }