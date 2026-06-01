#include "PluginEditor.h"

ModulatedStripEditor::ModulatedStripEditor(
    ModulatedStripProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(1100, 580);

    // INPUT
    setupKnob(inputGainSlider, inputGainLabel, "INPUT");

    // SATURATION
    setupKnob(driveSlider, driveLabel, "DRIVE");
    setupKnob(satMixSlider, satMixLabel, "SAT MIX");

    satModelSelector.addItemList({
        "NEVE", "SSL", "API", "TUBE", 
        "TAPE", "FET", "IRON"}, 1);
    satModelSelector.setColour(
        juce::ComboBox::backgroundColourId,
        juce::Colour(0xFF1A1A1A));
    satModelSelector.setColour(
        juce::ComboBox::textColourId,
        juce::Colour(0xFFE8A838));
    satModelSelector.setColour(
        juce::ComboBox::outlineColourId,
        juce::Colour(0xFF3A3A3A));
    addAndMakeVisible(satModelSelector);

    // COMPRESSOR
    setupKnob(thresholdSlider, thresholdLabel, "THRESH");
    setupKnob(ratioSlider, ratioLabel, "RATIO");
    setupKnob(attackSlider, attackLabel, "ATTACK");
    setupKnob(releaseSlider, releaseLabel, "RELEASE");
    setupKnob(makeupSlider, makeupLabel, "MAKEUP");
    setupKnob(compMixSlider, compMixLabel, "COMP MIX");
    setupKnob(kneeSlider, kneeLabel, "KNEE");

    compModelSelector.addItemList({
        "SSL Bus", "Fairchild 670", "LA-2A", 
        "1176", "API 2500"}, 1);
    compModelSelector.setColour(
        juce::ComboBox::backgroundColourId,
        juce::Colour(0xFF1A1A1A));
    compModelSelector.setColour(
        juce::ComboBox::textColourId,
        juce::Colour(0xFFE8A838));
    compModelSelector.setColour(
        juce::ComboBox::outlineColourId,
        juce::Colour(0xFF3A3A3A));
    addAndMakeVisible(compModelSelector);

    // EQ
    setupKnob(eqLowGainSlider, eqLowGainLabel, "LOW");
    setupKnob(eqLowFreqSlider, eqLowFreqLabel, "LOW FRQ");
    setupKnob(eqMidGainSlider, eqMidGainLabel, "MID");
    setupKnob(eqMidFreqSlider, eqMidFreqLabel, "MID FRQ");
    setupKnob(eqMidQSlider, eqMidQLabel, "MID Q");
    setupKnob(eqHighGainSlider, eqHighGainLabel, "HIGH");
    setupKnob(eqHighFreqSlider, eqHighFreqLabel, "HI FRQ");
    setupKnob(eqHPFSlider, eqHPFLabel, "HPF");

    eqModelSelector.addItemList({
        "Neve 1073", "Neve 1084", "SSL 4000E", 
        "Pultec EQP-1A", "API 550A"}, 1);
    eqModelSelector.setColour(
        juce::ComboBox::backgroundColourId,
        juce::Colour(0xFF1A1A1A));
    eqModelSelector.setColour(
        juce::ComboBox::textColourId,
        juce::Colour(0xFFE8A838));
    eqModelSelector.setColour(
        juce::ComboBox::outlineColourId,
        juce::Colour(0xFF3A3A3A));
    addAndMakeVisible(eqModelSelector);

    // OUTPUT
    setupKnob(outputGainSlider, outputGainLabel, "OUTPUT");

    //──────────────────────────────────────────
    // ALL ATTACHMENTS
    //──────────────────────────────────────────

    inputGainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "inputGain", inputGainSlider);

    driveAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "drive", driveSlider);
    satMixAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "satMix", satMixSlider);
    satModelAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.apvts, "satModel", satModelSelector);

    thresholdAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "compThreshold", thresholdSlider);
    ratioAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "compRatio", ratioSlider);
    attackAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "compAttack", attackSlider);
    releaseAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "compRelease", releaseSlider);
    makeupAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "compMakeup", makeupSlider);
    compMixAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "compMix", compMixSlider);
    kneeAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "compKnee", kneeSlider);
    compModelAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.apvts, "compModel", compModelSelector);

    eqLowGainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "eqLowGain", eqLowGainSlider);
    eqLowFreqAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "eqLowFreq", eqLowFreqSlider);
    eqMidGainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "eqMidGain", eqMidGainSlider);
    eqMidFreqAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "eqMidFreq", eqMidFreqSlider);
    eqMidQAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "eqMidQ", eqMidQSlider);
    eqHighGainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "eqHighGain", eqHighGainSlider);
    eqHighFreqAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "eqHighFreq", eqHighFreqSlider);
    eqHPFAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "eqHPF", eqHPFSlider);
    eqModelAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.apvts, "eqModel", eqModelSelector);

    outputGainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "outputGain", outputGainSlider);
}

ModulatedStripEditor::~ModulatedStripEditor() {}

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

    label.setText(text, juce::dontSendNotification);
    label.setColour(
        juce::Label::textColourId,
        juce::Colour(0xFFE8C878));
    label.setJustificationType(
        juce::Justification::centred);
    label.setFont(juce::Font(juce::FontOptions(9.0f)));
    addAndMakeVisible(label);
}

void ModulatedStripEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF0A0A0A));

    // Title
    g.setColour(juce::Colour(0xFFE8A838));
    g.setFont(juce::Font(
        juce::FontOptions(26.0f).withStyle("Bold")));
    g.drawText("MODULATED STRIP",
        0, 0, getWidth(), 45,
        juce::Justification::centred);

    g.setColour(juce::Colour(0xFF3A3A3A));
    g.drawHorizontalLine(45, 15.0f, getWidth() - 15.0f);

    auto sectionFont = juce::Font(
        juce::FontOptions(10.0f).withStyle("Bold"));

    // INPUT
    g.setColour(juce::Colour(0xFF111111));
    g.fillRoundedRectangle(8, 52, 90, 510, 5.0f);
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawRoundedRectangle(8, 52, 90, 510, 5.0f, 1.0f);
    g.setColour(juce::Colour(0xFFE8A838));
    g.setFont(sectionFont);
    g.drawText("INPUT", 8, 55, 90, 16,
        juce::Justification::centred);

    // SATURATION
    g.setColour(juce::Colour(0xFF111111));
    g.fillRoundedRectangle(103, 52, 195, 510, 5.0f);
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawRoundedRectangle(103, 52, 195, 510, 5.0f, 1.0f);
    g.setColour(juce::Colour(0xFFE8A838));
    g.setFont(sectionFont);
    g.drawText("SATURATION", 103, 55, 195, 16,
        juce::Justification::centred);

    // COMPRESSOR
    g.setColour(juce::Colour(0xFF111111));
    g.fillRoundedRectangle(303, 52, 295, 510, 5.0f);
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawRoundedRectangle(303, 52, 295, 510, 5.0f, 1.0f);
    g.setColour(juce::Colour(0xFFE8A838));
    g.setFont(sectionFont);
    g.drawText("COMPRESSOR", 303, 55, 295, 16,
        juce::Justification::centred);

    // EQ
    g.setColour(juce::Colour(0xFF111111));
    g.fillRoundedRectangle(603, 52, 395, 510, 5.0f);
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawRoundedRectangle(603, 52, 395, 510, 5.0f, 1.0f);
    g.setColour(juce::Colour(0xFFE8A838));
    g.setFont(sectionFont);
    g.drawText("EQUALIZER", 603, 55, 395, 16,
        juce::Justification::centred);

    // OUTPUT
    g.setColour(juce::Colour(0xFF111111));
    g.fillRoundedRectangle(1003, 52, 90, 510, 5.0f);
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawRoundedRectangle(1003, 52, 90, 510, 5.0f, 1.0f);
    g.setColour(juce::Colour(0xFFE8A838));
    g.setFont(sectionFont);
    g.drawText("OUTPUT", 1003, 55, 90, 16,
        juce::Justification::centred);
}

void ModulatedStripEditor::resized()
{
    int k = 75;   // knob size
    int lh = 14;  // label height

    // INPUT
    inputGainLabel.setBounds(15, 75, k, lh);
    inputGainSlider.setBounds(15, 89, k, k);

    // SATURATION
    satModelSelector.setBounds(113, 75, 175, 25);
    driveLabel.setBounds(113, 110, k, lh);
    driveSlider.setBounds(113, 124, k, k);
    satMixLabel.setBounds(208, 110, k, lh);
    satMixSlider.setBounds(208, 124, k, k);

    // COMPRESSOR
    compModelSelector.setBounds(313, 75, 175, 25);

    thresholdLabel.setBounds(313, 110, k, lh);
    thresholdSlider.setBounds(313, 124, k, k);
    ratioLabel.setBounds(403, 110, k, lh);
    ratioSlider.setBounds(403, 124, k, k);
    kneeLabel.setBounds(493, 110, k, lh);
    kneeSlider.setBounds(493, 124, k, k);

    attackLabel.setBounds(313, 215, k, lh);
    attackSlider.setBounds(313, 229, k, k);
    releaseLabel.setBounds(403, 215, k, lh);
    releaseSlider.setBounds(403, 229, k, k);

    makeupLabel.setBounds(313, 320, k, lh);
    makeupSlider.setBounds(313, 334, k, k);
    compMixLabel.setBounds(403, 320, k, lh);
    compMixSlider.setBounds(403, 334, k, k);

    // EQ
    eqModelSelector.setBounds(613, 75, 175, 25);

    eqLowGainLabel.setBounds(613, 110, k, lh);
    eqLowGainSlider.setBounds(613, 124, k, k);
    eqLowFreqLabel.setBounds(703, 110, k, lh);
    eqLowFreqSlider.setBounds(703, 124, k, k);

    eqMidGainLabel.setBounds(613, 215, k, lh);
    eqMidGainSlider.setBounds(613, 229, k, k);
    eqMidFreqLabel.setBounds(703, 215, k, lh);
    eqMidFreqSlider.setBounds(703, 229, k, k);
    eqMidQLabel.setBounds(793, 215, k, lh);
    eqMidQSlider.setBounds(793, 229, k, k);

    eqHighGainLabel.setBounds(613, 320, k, lh);
    eqHighGainSlider.setBounds(613, 334, k, k);
    eqHighFreqLabel.setBounds(703, 320, k, lh);
    eqHighFreqSlider.setBounds(703, 334, k, k);

    eqHPFLabel.setBounds(613, 425, k, lh);
    eqHPFSlider.setBounds(613, 439, k, k);

    // OUTPUT
    outputGainLabel.setBounds(1013, 75, k, lh);
    outputGainSlider.setBounds(1013, 89, k, k);
}