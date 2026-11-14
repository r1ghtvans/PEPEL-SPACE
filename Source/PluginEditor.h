#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PepelSpaceLookAndFeel.h"

class PepelSpaceAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PepelSpaceAudioProcessorEditor(PepelSpaceAudioProcessor&);
    ~PepelSpaceAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawVHSEffects(juce::Graphics& g);

    PepelSpaceAudioProcessor& proc;
    PepelSpaceLookAndFeel vhsLnf;

    juce::Image bgImage;

    juce::Slider sIn, sOut, sMix, sWidth, sMBFreq, sMBAmt, sMid, sSide,
                 sPan, sDepth, sHeight, sDL, sDR;
    juce::ToggleButton bBypass, bDelayOn, bPhL, bPhR, bSwap, bMono;
    juce::Label lblCorr, lblLevels;

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SA> aIn, aOut, aMix, aWidth, aMBFreq, aMBAmt, aMid, aSide,
                        aPan, aDepth, aHeight, aDL, aDR;
    std::unique_ptr<BA> aBypass, aDelayOn, aPhL, aPhR, aSwap, aMono;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PepelSpaceAudioProcessorEditor)
};