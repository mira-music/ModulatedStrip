#include "PluginEditor.h"

ModulatedStripEditor::ModulatedStripEditor(
    ModulatedStripProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    // Set plugin window size
    setSize(900, 550);

    // Setup the gain slider
    inputGainSlider.setSliderStyle(
        juce::Slider::RotaryHorizontalVerticalDrag);
    inputGainSlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 80, 20);
    inputGainSlider.setColour(
        juce::Slider::rotarySliderFillColourId,
        juce::Colour(0xFFE8A838)); // Amber color
    inputGainSlider.setColour(
        juce::Slider::rotarySliderOutlineColourId,
        juce::Colour(0xFF2A2A2A)); // Dark outline
    addAndMakeVisible(inputGainSlider);

    // Setup the label
    inputGainLabel.setText("INPUT GAIN", 
        juce::dontSendNotification);
    inputGainLabel.setColour(
        juce::Label::textColourId,
        juce::Colour(0xFFE8C878)); // Warm cream
    inputGainLabel.setJustificationType(
        juce::Justification::centred);
    addAndMakeVisible(inputGainLabel);

    // Connect slider to parameter
    inputGainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.apvts, 
            "inputGain", 
            inputGainSlider);
}

ModulatedStripEditor::~ModulatedStripEditor() {}

void ModulatedStripEditor::paint(juce::Graphics& g)
{
    // Black background
    g.fillAll(juce::Colour(0xFF0A0A0A));

    // Draw plugin name
    g.setColour(juce::Colour(0xFFE8A838)); // Amber
    g.setFont(juce::Font(juce::FontOptions(32.0f).withStyle("Bold")));
    g.drawText("MODULATED STRIP",
               getLocalBounds().removeFromTop(60),
               juce::Justification::centred);

    // Draw subtle line under title
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawHorizontalLine(65, 20.0f, 
                         (float)getWidth() - 20.0f);
}

void ModulatedStripEditor::resized()
{
    // Position the label above the knob
    inputGainLabel.setBounds(
        getWidth()/2 - 60,   // X position (centered)
        80,                   // Y position
        120,                  // Width
        20);                  // Height

    // Position the knob below the label
    inputGainSlider.setBounds(
        getWidth()/2 - 60,   // X position (centered)
        100,                  // Y position
        120,                  // Width
        120);                 // Height
}