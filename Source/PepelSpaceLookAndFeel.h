#pragma once
#include <JuceHeader.h>

class PepelSpaceLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PepelSpaceLookAndFeel();
    ~PepelSpaceLookAndFeel() override;

    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override;
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    juce::Font getVHSFont(float height);
    juce::Font getDoppelgangerFont(float height);

private:
    juce::Typeface::Ptr vhsTypeface;
    juce::Typeface::Ptr doppelgangerTypeface;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PepelSpaceLookAndFeel)
};