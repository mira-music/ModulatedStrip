#include "PluginEditor.h"

ModulatedStripEditor::ModulatedStripEditor(
    ModulatedStripProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(900, 550);

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

    // ATTACHMENTS
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
        juce::Slider::TextBoxBelow, false, 70, 18);
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
    label.setFont(juce::Font(juce::FontOptions(10.0f)));
    addAndMakeVisible(label);
}

void ModulatedStripEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF0A0A0A));

    g.setColour(juce::Colour(0xFFE8A838));
    g.setFont(juce::Font(
        juce::FontOptions(28.0f).withStyle("Bold")));
    g.drawText("MODULATED STRIP",
        0, 0, getWidth(), 50,
        juce::Justification::centred);

    g.setColour(juce::Colour(0xFF3A3A3A));
    g.drawHorizontalLine(50, 20.0f, getWidth() - 20.0f);

    auto sectionFont = juce::Font(
        juce::FontOptions(11.0f).withStyle("Bold"));

    // INPUT PANEL
    g.setColour(juce::Colour(0xFF111111));
    g.fillRoundedRectangle(10, 60, 120, 470, 6.0f);
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawRoundedRectangle(10, 60, 120, 470, 6.0f, 1.0f);
    g.setColour(juce::Colour(0xFFE8A838));
    g.setFont(sectionFont);
    g.drawText("INPUT", 10, 64, 120, 20,
        juce::Justification::centred);

    // SATURATION PANEL
    g.setColour(juce::Colour(0xFF111111));
    g.fillRoundedRectangle(140, 60, 230, 470, 6.0f);
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawRoundedRectangle(140, 60, 230, 470, 6.0f, 1.0f);
    g.setColour(juce::Colour(0xFFE8A838));
    g.setFont(sectionFont);
    g.drawText("SATURATION", 140, 64, 230, 20,
        juce::Justification::centred);

    // COMPRESSOR PANEL
    g.setColour(juce::Colour(0xFF111111));
    g.fillRoundedRectangle(380, 60, 510, 470, 6.0f);
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawRoundedRectangle(380, 60, 510, 470, 6.0f, 1.0f);
    g.setColour(juce::Colour(0xFFE8A838));
    g.setFont(sectionFont);
    g.drawText("COMPRESSOR", 380, 64, 510, 20,
        juce::Justification::centred);
}

void ModulatedStripEditor::resized()
{
    int knobSize = 85;
    int labelH = 16;

    // INPUT
    inputGainLabel.setBounds(25, 90, knobSize, labelH);
    inputGainSlider.setBounds(25, 106, knobSize, knobSize);

    // SATURATION
    satModelSelector.setBounds(155, 90, 200, 28);
    driveLabel.setBounds(155, 135, knobSize, labelH);
    driveSlider.setBounds(155, 151, knobSize, knobSize);
    satMixLabel.setBounds(260, 135, knobSize, labelH);
    satMixSlider.setBounds(260, 151, knobSize, knobSize);

    // COMPRESSOR
    compModelSelector.setBounds(395, 90, 200, 28);

    // Row 1
    thresholdLabel.setBounds(395, 135, knobSize, labelH);
    thresholdSlider.setBounds(395, 151, knobSize, knobSize);
    ratioLabel.setBounds(495, 135, knobSize, labelH);
    ratioSlider.setBounds(495, 151, knobSize, knobSize);
    kneeLabel.setBounds(595, 135, knobSize, labelH);
    kneeSlider.setBounds(595, 151, knobSize, knobSize);

    // Row 2
    attackLabel.setBounds(395, 260, knobSize, labelH);
    attackSlider.setBounds(395, 276, knobSize, knobSize);
    releaseLabel.setBounds(495, 260, knobSize, labelH);
    releaseSlider.setBounds(495, 276, knobSize, knobSize);

    // Row 3
    makeupLabel.setBounds(395, 385, knobSize, labelH);
    makeupSlider.setBounds(395, 401, knobSize, knobSize);
    compMixLabel.setBounds(495, 385, knobSize, labelH);
    compMixSlider.setBounds(495, 401, knobSize, knobSize);
}