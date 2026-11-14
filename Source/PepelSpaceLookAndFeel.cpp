#include "PepelSpaceLookAndFeel.h"

PepelSpaceLookAndFeel::PepelSpaceLookAndFeel()
{
    // Загрузка шрифтов из бинарных данных (имена файлов должны совпадать с CMake)
    vhsTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::VHSttf, BinaryData::VHSttfSize);
    doppelgangerTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::DoppelgangerRUSttf, BinaryData::DoppelgangerRUSttfSize);

    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff4da3ff)); // Неоновый синий
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a2a33));
    setColour(juce::Label::textColourId, juce::Colour(0xffE0E0E0));
}

PepelSpaceLookAndFeel::~PepelSpaceLookAndFeel() {}

juce::Font PepelSpaceLookAndFeel::getVHSFont(float height)
{
    if (vhsTypeface) return juce::Font(vhsTypeface).withHeight(height);
    return juce::Font("Courier New", height, juce::Font::plain);
}

juce::Font PepelSpaceLookAndFeel::getDoppelgangerFont(float height)
{
    if (doppelgangerTypeface) return juce::Font(doppelgangerTypeface).withHeight(height);
    return juce::Font("Arial", height, juce::Font::bold);
}

juce::Typeface::Ptr PepelSpaceLookAndFeel::getTypefaceForFont(const juce::Font& font)
{
    // По умолчанию используем VHS шрифт для интерфейса
    if (font.getTypefaceName() == "VHS") return vhsTypeface;
    if (font.getTypefaceName() == "Doppelganger") return doppelgangerTypeface;
    return juce::LookAndFeel_V4::getTypefaceForFont(font);
}

void PepelSpaceLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                             float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(5.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Темный металлический фон ручки
    g.setColour(juce::Colour(0xff1A1B22));
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
    
    // Обводка (потертости)
    g.setColour(juce::Colour(0xff333333));
    g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 2.0f);

    // Неоновая дуга
    juce::Path p;
    p.addCentredArc(centreX, centreY, radius - 4.0f, radius - 4.0f, 0.0f, rotaryStartAngle, angle, true);
    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
    g.strokePath(p, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Указатель
    juce::Path pointer;
    pointer.addRectangle(-1.5f, -radius + 4.0f, 3.0f, radius * 0.4f);
    g.setColour(juce::Colours::white);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
}

void PepelSpaceLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
    auto isOn = button.getToggleState();

    // Фон кнопки
    g.setColour(isOn ? juce::Colour(0xff4da3ff).withAlpha(0.3f) : juce::Colour(0xff1A1B22));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Рамка (красная если включена)
    g.setColour(isOn ? juce::Colour(0xffFF3333) : juce::Colour(0xff333333));
    g.drawRoundedRectangle(bounds, 4.0f, 1.5f);

    // Текст
    g.setColour(isOn ? juce::Colours::white : juce::Colours::grey);
    g.setFont(getVHSFont(14.0f));
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred, false);
}