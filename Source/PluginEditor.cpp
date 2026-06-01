#include "PluginEditor.h"

ModulatedStripEditor::ModulatedStripEditor(
    ModulatedStripProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(900, 550);

    // Setup all knobs
    setupKnob(inputGainSlider, inputGainLabel, 
              "INPUT GAIN");
    setupKnob(driveSlider, driveLabel, 
              "DRIVE");
    setupKnob(satMixSlider, satMixLabel, 
              "SAT MIX");

    // Setup model selector
    satModelLabel.setText("SATURATION", 
        juce::dontSendNotification);
    satModelLabel.setColour(
        juce::Label::textColourId,
        juce::Colour(0xFFE8A838));
    satModelLabel.setJustificationType(
        juce::Justification::centred);
    addAndMakeVisible(satModelLabel);

    satModelSelector.addItemList({
        "NEVE",
        "SSL",
        "API",
        "TUBE",
        "TAPE",
        "FET",
        "IRON"
    }, 1);
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

    // Connect controls to parameters
    inputGainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
            processor.apvts, 
            "inputGain", 
            inputGainSlider);

    driveAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
            processor.apvts, 
            "drive", 
            driveSlider);

    satMixAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::SliderAttachment>(
            processor.apvts, 
            "satMix", 
            satMixSlider);

    satModelAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState
        ::ComboBoxAttachment>(
            processor.apvts, 
            "satModel", 
            satModelSelector);
}

ModulatedStripEditor::~ModulatedStripEditor() {}

void ModulatedStripEditor::setupKnob(
    juce::Slider& slider,
    juce::Label& label,
    const juce::String& labelText)
{
    // Knob style
    slider.setSliderStyle(
        juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 80, 20);

    // Amber color scheme
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

    // Label style
    label.setText(labelText, 
        juce::dontSendNotification);
    label.setColour(
        juce::Label::textColourId,
        juce::Colour(0xFFE8C878));
    label.setJustificationType(
        juce::Justification::centred);
    label.setFont(juce::Font(
        juce::FontOptions(11.0f)));
    addAndMakeVisible(label);
}

void ModulatedStripEditor::paint(juce::Graphics& g)
{
    // Black background
    g.fillAll(juce::Colour(0xFF0A0A0A));

    // Title
    g.setColour(juce::Colour(0xFFE8A838));
    g.setFont(juce::Font(
        juce::FontOptions(28.0f).withStyle("Bold")));
    g.drawText("MODULATED STRIP",
               0, 0, getWidth(), 50,
               juce::Justification::centred);

    // Title underline
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawHorizontalLine(52, 20.0f, 
                         getWidth() - 20.0f);

    // INPUT section background
    g.setColour(juce::Colour(0xFF111111));
    g.fillRoundedRectangle(10, 60, 160, 460, 6.0f);
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawRoundedRectangle(10, 60, 160, 460, 6.0f, 
                            1.0f);

    // INPUT label
    g.setColour(juce::Colour(0xFFE8A838));
    g.setFont(juce::Font(
        juce::FontOptions(11.0f).withStyle("Bold")));
    g.drawText("INPUT", 10, 65, 160, 20,
               juce::Justification::centred);

    // SATURATION section background
    g.setColour(juce::Colour(0xFF111111));
    g.fillRoundedRectangle(180, 60, 340, 460, 6.0f);
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawRoundedRectangle(180, 60, 340, 460, 6.0f, 
                            1.0f);

    // SATURATION label
    g.setColour(juce::Colour(0xFFE8A838));
    g.drawText("SATURATION", 180, 65, 340, 20,
               juce::Justification::centred);
}

void ModulatedStripEditor::resized()
{
    // INPUT GAIN knob
    inputGainLabel.setBounds(30, 85, 120, 20);
    inputGainSlider.setBounds(30, 105, 120, 120);

    // SATURATION MODEL selector
    satModelLabel.setBounds(200, 85, 300, 20);
    satModelSelector.setBounds(220, 108, 260, 30);

    // DRIVE knob
    driveLabel.setBounds(200, 155, 120, 20);
    driveSlider.setBounds(200, 175, 120, 120);

    // SAT MIX knob
    satMixLabel.setBounds(360, 155, 120, 20);
    satMixSlider.setBounds(360, 175, 120, 120);
}