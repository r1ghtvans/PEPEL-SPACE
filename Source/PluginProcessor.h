#pragma once
#include <JuceHeader.h>

class PepelSpaceAudioProcessor : public juce::AudioProcessor
{
public:
    PepelSpaceAudioProcessor();
    ~PepelSpaceAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Pepel Space"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.05; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

    // Данные для UI (Анализатор)
    std::atomic<float> currentCorrelation { 1.0f };
    std::atomic<float> currentLevelL { 0.0f };
    std::atomic<float> currentLevelR { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    double currentSampleRate = 44100.0;
    
    juce::SmoothedValue<float> smoothedInGain, smoothedOutGain, smoothedDryWet, smoothedWidth;
    float mbLP_L = 0.0f, mbLP_R = 0.0f;
    float depthLP_L = 0.0f, depthLP_R = 0.0f;
    float heightLP_L = 0.0f, heightLP_R = 0.0f;

    juce::AudioBuffer<float> delayBufL, delayBufR;
    int delayWritePos = 0, delayBufSize = 0;
    juce::AudioBuffer<float> dryBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PepelSpaceAudioProcessor)
};