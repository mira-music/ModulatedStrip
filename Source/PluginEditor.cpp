#include "PluginEditor.h"

ModulatedStripEditor::ModulatedStripEditor(
    ModulatedStripProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(1100, 600);

    //──────────────────────────────────────────────
    // INPUT
    //──────────────────────────────────────────────
    setupKnob(inputGainSlider, inputGainLabel, "INPUT");

    //──────────────────────────────────────────────
    // SATURATION
    //──────────────────────────────────────────────
    setupKnob(driveSlider,  driveLabel,  "DRIVE");
    setupKnob(satMixSlider, satMixLabel, "SAT MIX");

    setupCombo(satModelSelector, {
        "NEVE", "SSL", "API", "TUBE",
        "TAPE", "FET", "IRON"});

    addAndMakeVisible(satBypassBtn);

    //──────────────────────────────────────────────
    // COMPRESSOR
    //──────────────────────────────────────────────
    setupKnob(thresholdSlider, thresholdLabel, "THRESH");
    setupKnob(ratioSlider,     ratioLabel,     "RATIO");
    setupKnob(attackSlider,    attackLabel,    "ATTACK");
    setupKnob(releaseSlider,   releaseLabel,   "RELEASE");
    setupKnob(makeupSlider,    makeupLabel,    "MAKEUP");
    setupKnob(compMixSlider,   compMixLabel,   "MIX");
    setupKnob(kneeSlider,      kneeLabel,      "KNEE");

    setupCombo(compModelSelector, {
        "SSL Bus", "Fairchild 670",
        "LA-2A", "1176", "API 2500"});

    addAndMakeVisible(compBypassBtn);

    //──────────────────────────────────────────────
    // EQ
    //──────────────────────────────────────────────
    setupKnob(eqLowGainSlider,  eqLowGainLabel,  "LOW");
    setupKnob(eqLowFreqSlider,  eqLowFreqLabel,  "LO FRQ");
    setupKnob(eqMidGainSlider,  eqMidGainLabel,  "MID");
    setupKnob(eqMidFreqSlider,  eqMidFreqLabel,  "MI FRQ");
    setupKnob(eqMidQSlider,     eqMidQLabel,     "MID Q");
    setupKnob(eqHighGainSlider, eqHighGainLabel, "HIGH");
    setupKnob(eqHighFreqSlider, eqHighFreqLabel, "HI FRQ");
    setupKnob(eqHPFSlider,      eqHPFLabel,      "HPF");

    setupCombo(eqModelSelector, {
        "Neve 1073", "Neve 1084", "SSL 4000E",
        "Pultec EQP-1A", "API 550A"});

    addAndMakeVisible(eqBypassBtn);
    addAndMakeVisible(eqPreCompBtn);

    //──────────────────────────────────────────────
    // OUTPUT
    //──────────────────────────────────────────────
    setupKnob(outputGainSlider, outputGainLabel, "OUTPUT");

    //──────────────────────────────────────────────
    // METERS
    //──────────────────────────────────────────────
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    addAndMakeVisible(grMeter);

    grMeter.setIsGainReduction(true);

    auto setupMeterLabel = [this](
        juce::Label& label,
        const juce::String& text)
    {
        label.setText(text,
            juce::dontSendNotification);
        label.setColour(
            juce::Label::textColourId,
            juce::Colour(0xFF888888));
        label.setFont(juce::Font(
            juce::FontOptions(9.0f)));
        label.setJustificationType(
            juce::Justification::centred);
        addAndMakeVisible(label);
    };

    setupMeterLabel(inputMeterLabel,  "IN");
    setupMeterLabel(outputMeterLabel, "OUT");
    setupMeterLabel(grMeterLabel,     "GR");

    //──────────────────────────────────────────────
    // ALL PARAMETER ATTACHMENTS
    //──────────────────────────────────────────────
    auto& avpts = processor.apvts;

    inputGainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "inputGain", inputGainSlider);

    driveAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "drive", driveSlider);
    satMixAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "satMix", satMixSlider);
    satModelAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::ComboBoxAttachment>(
        avpts, "satModel", satModelSelector);
    satBypassAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::ButtonAttachment>(
        avpts, "satBypass", satBypassBtn);

    thresholdAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "compThreshold", thresholdSlider);
    ratioAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "compRatio", ratioSlider);
    attackAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "compAttack", attackSlider);
    releaseAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "compRelease", releaseSlider);
    makeupAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "compMakeup", makeupSlider);
    compMixAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "compMix", compMixSlider);
    kneeAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "compKnee", kneeSlider);
    compModelAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::ComboBoxAttachment>(
        avpts, "compModel", compModelSelector);
    compBypassAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::ButtonAttachment>(
        avpts, "compBypass", compBypassBtn);

    eqLowGainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "eqLowGain", eqLowGainSlider);
    eqLowFreqAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "eqLowFreq", eqLowFreqSlider);
    eqMidGainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "eqMidGain", eqMidGainSlider);
    eqMidFreqAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "eqMidFreq", eqMidFreqSlider);
    eqMidQAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "eqMidQ", eqMidQSlider);
    eqHighGainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "eqHighGain", eqHighGainSlider);
    eqHighFreqAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "eqHighFreq", eqHighFreqSlider);
    eqHPFAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "eqHPF", eqHPFSlider);
    eqModelAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::ComboBoxAttachment>(
        avpts, "eqModel", eqModelSelector);
    eqBypassAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::ButtonAttachment>(
        avpts, "eqBypass", eqBypassBtn);
    eqPreCompAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::ButtonAttachment>(
        avpts, "eqPreComp", eqPreCompBtn);

    outputGainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
        avpts, "outputGain", outputGainSlider);

    // Start meter timer at 30Hz
    startTimerHz(30);
}

ModulatedStripEditor::~ModulatedStripEditor()
{
    stopTimer();
}

//==============================================================================
// TIMER - Updates meters at 30Hz
//==============================================================================
void ModulatedStripEditor::timerCallback()
{
    inputMeter .setLevel(processor.getInputPeak());
    outputMeter.setLevel(processor.getOutputPeak());

    // GR meter - convert negative dB to positive
    float gr = std::abs(processor.getGainReduction());
    float grLinear = juce::Decibels::decibelsToGain(
        -gr, -60.0f);
    grMeter.setLevel(1.0f - grLinear);
}

//==============================================================================
// SETUP HELPERS
//==============================================================================
void ModulatedStripEditor::setupKnob(
    juce::Slider& slider,
    juce::Label& label,
    const juce::String& text)
{
    slider.setSliderStyle(
        juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 65, 16);
    slider.setColour(
        juce::Slider::rotarySliderFillColourId,
        juce::Colour(0xFFE8A838));
    slider.setColour(
        juce::Slider::rotarySliderOutlineColourId,
        juce::Colour(0xFF2A2A2A));
    slider.setColour(
        juce::Slider::textBoxTextColourId,
        juce::Colour(0xFFE8C878));
    slider.setColour(
        juce::Slider::textBoxBackgroundColourId,
        juce::Colour(0xFF0A0A0A));
    slider.setColour(
        juce::Slider::textBoxOutlineColourId,
        juce::Colour(0xFF0A0A0A));
    addAndMakeVisible(slider);

    label.setText(text,
        juce::dontSendNotification);
    label.setColour(
        juce::Label::textColourId,
        juce::Colour(0xFFE8C878));
    label.setJustificationType(
        juce::Justification::centred);
    label.setFont(juce::Font(
        juce::FontOptions(9.0f)));
    addAndMakeVisible(label);
}

void ModulatedStripEditor::setupCombo(
    juce::ComboBox& box,
    const juce::StringArray& items)
{
    box.addItemList(items, 1);
    box.setColour(
        juce::ComboBox::backgroundColourId,
        juce::Colour(0xFF1A1A1A));
    box.setColour(
        juce::ComboBox::textColourId,
        juce::Colour(0xFFE8A838));
    box.setColour(
        juce::ComboBox::outlineColourId,
        juce::Colour(0xFF3A3A3A));
    box.setColour(
        juce::ComboBox::arrowColourId,
        juce::Colour(0xFFE8A838));
    addAndMakeVisible(box);
}

//==============================================================================
// PAINT - Background and section panels
//==============================================================================
void ModulatedStripEditor::paint(juce::Graphics& g)
{
    // Matte black background
    g.fillAll(juce::Colour(0xFF0A0A0A));

    // Title
    g.setColour(juce::Colour(0xFFE8A838));
    g.setFont(juce::Font(
        juce::FontOptions(24.0f).withStyle("Bold")));
    g.drawText("MODULATED STRIP",
        0, 0, getWidth(), 42,
        juce::Justification::centred);

    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawHorizontalLine(42,
        15.0f, getWidth() - 15.0f);

    auto sf = juce::Font(
        juce::FontOptions(10.0f).withStyle("Bold"));

    auto drawPanel = [&](float x, float y,
                         float w, float h,
                         const juce::String& title)
    {
        g.setColour(juce::Colour(0xFF111111));
        g.fillRoundedRectangle(x, y, w, h, 5.0f);
        g.setColour(juce::Colour(0xFF2A2A2A));
        g.drawRoundedRectangle(x, y, w, h, 5.0f, 1.0f);
        g.setColour(juce::Colour(0xFFE8A838));
        g.setFont(sf);
        g.drawText(title,
            static_cast<int>(x),
            static_cast<int>(y) + 3,
            static_cast<int>(w), 16,
            juce::Justification::centred);
    };

    drawPanel(  8,  48,  95, 540, "INPUT");
    drawPanel(108,  48, 200, 540, "SATURATION");
    drawPanel(313,  48, 310, 540, "COMPRESSOR");
    drawPanel(628,  48, 400, 540, "EQUALIZER");
    drawPanel(1033, 48,  60, 540, "OUT");

    // Meter labels area
    g.setColour(juce::Colour(0xFF111111));
    g.fillRoundedRectangle(8, 595, 1085, 5, 2.0f);
}

//==============================================================================
// RESIZED - Position all controls
//==============================================================================
void ModulatedStripEditor::resized()
{
    int k  = 72;  // knob size
    int lh = 14;  // label height

    //──────────────────────────────────────────────
    // INPUT SECTION
    //──────────────────────────────────────────────
    int ix = 18;
    int iy = 72;

    inputGainLabel .setBounds(ix, iy,      k, lh);
    inputGainSlider.setBounds(ix, iy + lh, k, k);

    inputMeterLabel.setBounds(ix, iy + lh + k + 8,
        k, lh);
    inputMeter     .setBounds(ix + 8,
        iy + lh + k + 8 + lh, k - 16, 300);

    //──────────────────────────────────────────────
    // SATURATION SECTION
    //──────────────────────────────────────────────
    int sx = 118;
    int sy = 68;

    satModelSelector.setBounds(sx, sy,       180, 24);
    satBypassBtn    .setBounds(sx + 140, sy + 28, 40, 20);

    sy += 55;
    driveLabel .setBounds(sx,      sy, k, lh);
    driveSlider.setBounds(sx,      sy + lh, k, k);
    satMixLabel .setBounds(sx + 90, sy, k, lh);
    satMixSlider.setBounds(sx + 90, sy + lh, k, k);

    //──────────────────────────────────────────────
    // COMPRESSOR SECTION
    //──────────────────────────────────────────────
    int cx = 323;
    int cy = 68;

    compModelSelector.setBounds(cx, cy,       190, 24);
    compBypassBtn    .setBounds(cx + 200, cy + 4, 40, 20);

    cy += 52;

    // Row 1 - Threshold Ratio Knee
    thresholdLabel .setBounds(cx,       cy, k, lh);
    thresholdSlider.setBounds(cx,       cy + lh, k, k);
    ratioLabel     .setBounds(cx + 95,  cy, k, lh);
    ratioSlider    .setBounds(cx + 95,  cy + lh, k, k);
    kneeLabel      .setBounds(cx + 190, cy, k, lh);
    kneeSlider     .setBounds(cx + 190, cy + lh, k, k);

    cy += k + lh + 18;

    // Row 2 - Attack Release
    attackLabel  .setBounds(cx,      cy, k, lh);
    attackSlider .setBounds(cx,      cy + lh, k, k);
    releaseLabel .setBounds(cx + 95, cy, k, lh);
    releaseSlider.setBounds(cx + 95, cy + lh, k, k);

    cy += k + lh + 18;

    // Row 3 - Makeup Mix
    makeupLabel  .setBounds(cx,      cy, k, lh);
    makeupSlider .setBounds(cx,      cy + lh, k, k);
    compMixLabel .setBounds(cx + 95, cy, k, lh);
    compMixSlider.setBounds(cx + 95, cy + lh, k, k);

    // GR meter
    grMeterLabel.setBounds(cx + 200, cy, k, lh);
    grMeter     .setBounds(cx + 218, cy + lh, 18, k + 60);

    //──────────────────────────────────────────────
    // EQ SECTION
    //──────────────────────────────────────────────
    int ex = 638;
    int ey = 68;

    eqModelSelector.setBounds(ex,       ey,       190, 24);
    eqBypassBtn    .setBounds(ex + 200, ey + 4,   40, 20);
    eqPreCompBtn   .setBounds(ex + 248, ey + 4,   55, 20);

    ey += 52;

    // Low band
    eqLowGainLabel .setBounds(ex,       ey, k, lh);
    eqLowGainSlider.setBounds(ex,       ey + lh, k, k);
    eqLowFreqLabel .setBounds(ex + 95,  ey, k, lh);
    eqLowFreqSlider.setBounds(ex + 95,  ey + lh, k, k);

    ey += k + lh + 18;

    // Mid band
    eqMidGainLabel .setBounds(ex,       ey, k, lh);
    eqMidGainSlider.setBounds(ex,       ey + lh, k, k);
    eqMidFreqLabel .setBounds(ex + 95,  ey, k, lh);
    eqMidFreqSlider.setBounds(ex + 95,  ey + lh, k, k);
    eqMidQLabel    .setBounds(ex + 190, ey, k, lh);
    eqMidQSlider   .setBounds(ex + 190, ey + lh, k, k);

    ey += k + lh + 18;

    // High band and HPF
    eqHighGainLabel .setBounds(ex,       ey, k, lh);
    eqHighGainSlider.setBounds(ex,       ey + lh, k, k);
    eqHighFreqLabel .setBounds(ex + 95,  ey, k, lh);
    eqHighFreqSlider.setBounds(ex + 95,  ey + lh, k, k);
    eqHPFLabel      .setBounds(ex + 285, ey, k, lh);
    eqHPFSlider     .setBounds(ex + 285, ey + lh, k, k);

    //──────────────────────────────────────────────
    // OUTPUT SECTION
    //──────────────────────────────────────────────
    int ox = 1040;
    int oy = 72;

    outputGainLabel .setBounds(ox, oy,      k, lh);
    outputGainSlider.setBounds(ox, oy + lh, k, k);

    outputMeterLabel.setBounds(ox,
        oy + lh + k + 8, k, lh);
    outputMeter     .setBounds(ox + 8,
        oy + lh + k + 8 + lh, k - 16, 300);
}