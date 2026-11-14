#include "PluginEditor.h"

PepelSpaceAudioProcessorEditor::PepelSpaceAudioProcessorEditor(PepelSpaceAudioProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    setLookAndFeel(&vhsLnf);
    setSize(1000, 750);

    // Загрузка фонового изображения (должно быть в Assets)
    bgImage = juce::ImageFileFormat::loadFrom(BinaryData::bg_png, BinaryData::bg_pngSize);

    auto setupKnob = [&](juce::Slider& s, std::unique_ptr<SA>& att, const char* id) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(s);
        att = std::make_unique<SA>(proc.apvts, id, s);
    };

    setupKnob(sIn, aIn, "input_gain");       setupKnob(sOut, aOut, "output_gain");
    setupKnob(sMix, aMix, "dry_wet");        setupKnob(sWidth, aWidth, "width");
    setupKnob(sMBFreq, aMBFreq, "mono_bass_freq"); setupKnob(sMBAmt, aMBAmt, "mono_bass_amount");
    setupKnob(sMid, aMid, "mid_gain");       setupKnob(sSide, aSide, "side_gain");
    setupKnob(sPan, aPan, "pan");            setupKnob(sDepth, aDepth, "depth");
    setupKnob(sHeight, aHeight, "height");   setupKnob(sDL, aDL, "delay_l");
    setupKnob(sDR, aDR, "delay_r");

    auto setupToggle = [&](juce::ToggleButton& b, const juce::String& t, std::unique_ptr<BA>& att, const char* id) {
        b.setButtonText(t);
        addAndMakeVisible(b);
        att = std::make_unique<BA>(proc.apvts, id, b);
    };
    setupToggle(bBypass, "Bypass", aBypass, "bypass");      setupToggle(bDelayOn, "Delay On", aDelayOn, "delay_on");
    setupToggle(bPhL, "Phase L", aPhL, "phase_invert_l");   setupToggle(bPhR, "Phase R", aPhR, "phase_invert_r");
    setupToggle(bSwap, "Swap L/R", aSwap, "channel_swap");  setupToggle(bMono, "Mono", aMono, "mono_check");

    lblCorr.setJustificationType(juce::Justification::centredLeft);
    lblCorr.setFont(vhsLnf.getVHSFont(22.0f));
    addAndMakeVisible(lblCorr);

    lblLevels.setJustificationType(juce::Justification::centredLeft);
    lblLevels.setFont(vhsLnf.getVHSFont(16.0f));
    addAndMakeVisible(lblLevels);

    startTimerHz(20);
}

PepelSpaceAudioProcessorEditor::~PepelSpaceAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void PepelSpaceAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0A0B10)); // Глубокий темный фон

    // Рисуем фон (если загрузился)
    if (bgImage.isValid())
    {
        g.setOpacity(0.4f); // Приглушаем картинку, чтобы текст читался
        g.drawImage(bgImage, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);
        g.setOpacity(1.0f);
    }

    // Отрисовка VHS эффектов и рамок
    drawVHSEffects(g);

    // Иероглифы вверху (Красный неон)
    g.setFont(vhsLnf.getDoppelgangerFont(40.0f));
    g.setColour(juce::Colour(0xffFF3333).withAlpha(0.8f));
    g.drawText("灰愛", getLocalBounds().removeFromTop(60), juce::Justification::centred, false);
    
    // Текст внизу справа
    g.setFont(vhsLnf.getVHSFont(18.0f));
    g.setColour(juce::Colour(0xff4da3ff).withAlpha(0.6f));
    g.drawText("by ПЕПЕL", getLocalBounds().removeFromBottom(40).removeFromRight(200), juce::Justification::centredRight, false);
}

void PepelSpaceAudioProcessorEditor::drawVHSEffects(juce::Graphics& g)
{
    // Эмуляция Scanlines (горизонтальные полосы)
    g.setColour(juce::Colours::black.withAlpha(0.1f));
    for (int i = 0; i < getHeight(); i += 3) {
        g.fillRect(0, i, getWidth(), 1);
    }

    // Случайный шум VHS
    juce::Random r;
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    for (int i = 0; i < 15; ++i) {
        g.fillRect(0, r.nextInt(getHeight()), getWidth(), 1);
    }

    // Рамки секций (стилизация под провода/цепи)
    g.setColour(juce::Colour(0xff2a2a33));
    auto drawSection = [&](juce::Rectangle<int> r, const juce::String& title) {
        g.drawRoundedRectangle(r.toFloat(), 4.0f, 1.0f);
        g.setColour(juce::Colour(0xffE0E0E0));
        g.setFont(vhsLnf.getDoppelgangerFont(16.0f));
        g.drawText(title, r.reduced(8, 4).removeFromTop(20), juce::Justification::centredLeft, false);
        g.setColour(juce::Colour(0xff2a2a33)); // Сброс цвета для следующей рамки
    };

    // Логические блоки
    drawSection({10, 70, 980, 90}, "I / O");
    drawSection({10, 170, 980, 100}, "STEREO WIDTH");
    drawSection({10, 280, 980, 100}, "3D POSITION");
    drawSection({10, 390, 980, 100}, "HAAS / DELAY");
    drawSection({10, 500, 980, 80}, "UTILITIES");
    drawSection({10, 590, 980, 150}, "ANALYSIS");
}

void PepelSpaceAudioProcessorEditor::resized()
{
    auto A = [](int x, int y, int w, int h) { return juce::Rectangle<int>(x, y, w, h); };

    sIn.setBounds(A(20, 90, 60, 60));   sOut.setBounds(A(90, 90, 60, 60));
    sMix.setBounds(A(160, 90, 60, 60)); bBypass.setBounds(A(240, 100, 100, 30));

    sWidth.setBounds(A(20, 190, 60, 60));  sMBFreq.setBounds(A(90, 190, 60, 60));
    sMBAmt.setBounds(A(160, 190, 60, 60)); sMid.setBounds(A(230, 190, 60, 60));
    sSide.setBounds(A(300, 190, 60, 60));

    sPan.setBounds(A(20, 300, 60, 60));  sDepth.setBounds(A(90, 300, 60, 60));
    sHeight.setBounds(A(160, 300, 60, 60));

    sDL.setBounds(A(20, 410, 60, 60));   sDR.setBounds(A(90, 410, 60, 60));
    bDelayOn.setBounds(A(160, 420, 100, 30));

    bPhL.setBounds(A(20, 520, 90, 30));  bPhR.setBounds(A(120, 520, 90, 30));
    bSwap.setBounds(A(220, 520, 100, 30)); bMono.setBounds(A(330, 520, 100, 30));

    lblCorr.setBounds(A(20, 620, 960, 40));
    lblLevels.setBounds(A(20, 660, 960, 40));
}

void PepelSpaceAudioProcessorEditor::timerCallback()
{
    const float c = proc.currentCorrelation.load();
    const float lL = proc.currentLevelL.load();
    const float lR = proc.currentLevelR.load();

    juce::String warn = (c < -0.05f) ? "  ⚠ ANTIPHASE" : "";
    lblCorr.setText("CORRELATION: " + juce::String(c, 2) + warn, juce::dontSendNotification);
    lblCorr.setColour(juce::Label::textColourId, c < -0.05f ? juce::Colour(0xffFF3333) : (c > 0.6f ? juce::Colour(0xff4da3ff) : juce::Colours::orange));

    lblLevels.setText("L: " + juce::String(lL, 2) + "   ||   R: " + juce::String(lR, 2), juce::dontSendNotification);

    repaint(); // Обновляем фон для эффекта VHS шума
}
