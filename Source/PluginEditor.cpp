#include "PluginEditor.h"

ModulatedStripEditor::ModulatedStripEditor(
    ModulatedStripProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(1100, 600);

    // Apply custom look and feel to entire plugin
    setLookAndFeel(&analogLAF);

    auto& apvts = processor.apvts;

    //──────────────────────────────────────────────
    // INPUT
    //──────────────────────────────────────────────
    inputGainKnob.setName("INPUT");
    addAndMakeVisible(inputGainKnob);

    //──────────────────────────────────────────────
    // SATURATION
    //──────────────────────────────────────────────
    driveKnob .setName("DRIVE");
    satMixKnob.setName("SAT MIX");
    addAndMakeVisible(driveKnob);
    addAndMakeVisible(satMixKnob);

    satModelSelector.addItemList({
        "NEVE", "SSL", "API", "TUBE",
        "TAPE", "FET", "IRON"}, 1);
    addAndMakeVisible(satModelSelector);
    addAndMakeVisible(satBypassBtn);

    //──────────────────────────────────────────────
    // COMPRESSOR
    //──────────────────────────────────────────────
    thresholdKnob.setName("THRESH");
    ratioKnob    .setName("RATIO");
    attackKnob   .setName("ATTACK");
    releaseKnob  .setName("RELEASE");
    makeupKnob   .setName("MAKEUP");
    compMixKnob  .setName("MIX");
    kneeKnob     .setName("KNEE");

    addAndMakeVisible(thresholdKnob);
    addAndMakeVisible(ratioKnob);
    addAndMakeVisible(attackKnob);
    addAndMakeVisible(releaseKnob);
    addAndMakeVisible(makeupKnob);
    addAndMakeVisible(compMixKnob);
    addAndMakeVisible(kneeKnob);

    compModelSelector.addItemList({
        "SSL Bus", "Fairchild 670",
        "LA-2A", "1176", "API 2500"}, 1);
    addAndMakeVisible(compModelSelector);
    addAndMakeVisible(compBypassBtn);

    // Model specific controls
    fairchildTCSelector.addItemList({
        "TC 1  (0.2ms / 0.3s)",
        "TC 2  (0.2ms / 0.8s)",
        "TC 3  (0.4ms / 2.0s)",
        "TC 4  (0.4ms / Auto)",
        "TC 5  (0.4ms / 5.0s)",
        "TC 6  (0.4ms / Auto fast)"}, 1);
    fairchildTCSelector.setVisible(false);
    addAndMakeVisible(fairchildTCSelector);

    allInBtn    .setVisible(false);
    thrustBtn   .setVisible(false);
    topologyBtn .setVisible(false);
    la2aLimitBtn.setVisible(false);
    addAndMakeVisible(allInBtn);
    addAndMakeVisible(thrustBtn);
    addAndMakeVisible(topologyBtn);
    addAndMakeVisible(la2aLimitBtn);

    compModelHintLabel.setFont(juce::Font(
        juce::FontOptions(8.0f)));
    compModelHintLabel.setColour(
        juce::Label::textColourId,
        juce::Colour(0xFF888888));
    compModelHintLabel.setJustificationType(
        juce::Justification::centred);
    addAndMakeVisible(compModelHintLabel);

    // Analog needle GR meter
    addAndMakeVisible(grNeedleMeter);

    //──────────────────────────────────────────────
    // EQ
    //──────────────────────────────────────────────
    eqLowGainKnob .setName("LOW");
    eqLowFreqKnob .setName("LO FRQ");
    eqMidGainKnob .setName("MID");
    eqMidFreqKnob .setName("MI FRQ");
    eqMidQKnob    .setName("MID Q");
    eqHighGainKnob.setName("HIGH");
    eqHighFreqKnob.setName("HI FRQ");
    eqHPFKnob     .setName("HPF");

    addAndMakeVisible(eqLowGainKnob);
    addAndMakeVisible(eqLowFreqKnob);
    addAndMakeVisible(eqMidGainKnob);
    addAndMakeVisible(eqMidFreqKnob);
    addAndMakeVisible(eqMidQKnob);
    addAndMakeVisible(eqHighGainKnob);
    addAndMakeVisible(eqHighFreqKnob);
    addAndMakeVisible(eqHPFKnob);

    eqModelSelector.addItemList({
        "Neve 1073", "Neve 1084", "SSL 4000E",
        "Pultec EQP-1A", "API 550A"}, 1);
    addAndMakeVisible(eqModelSelector);
    addAndMakeVisible(eqBypassBtn);
    addAndMakeVisible(eqPreCompBtn);

    eqModelHintLabel.setFont(juce::Font(
        juce::FontOptions(8.0f)));
    eqModelHintLabel.setColour(
        juce::Label::textColourId,
        juce::Colour(0xFF888888));
    eqModelHintLabel.setJustificationType(
        juce::Justification::centred);
    addAndMakeVisible(eqModelHintLabel);

    //──────────────────────────────────────────────
    // OUTPUT
    //──────────────────────────────────────────────
    outputGainKnob.setName("OUTPUT");
    addAndMakeVisible(outputGainKnob);

    //──────────────────────────────────────────────
    // METERS
    //──────────────────────────────────────────────
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);

    auto setupLabel = [this](juce::Label& label,
                             const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId,
            juce::Colour(0xFF888888));
        label.setFont(juce::Font(
            juce::FontOptions(8.0f)));
        label.setJustificationType(
            juce::Justification::centred);
        addAndMakeVisible(label);
    };

    setupLabel(inputMeterLabel,  "IN");
    setupLabel(outputMeterLabel, "OUT");

    //──────────────────────────────────────────────
    // SCREWS
    //──────────────────────────────────────────────
    screwTL.setRotation(0.3f);
    screwTR.setRotation(1.1f);
    screwBL.setRotation(0.7f);
    screwBR.setRotation(2.1f);
    addAndMakeVisible(screwTL);
    addAndMakeVisible(screwTR);
    addAndMakeVisible(screwBL);
    addAndMakeVisible(screwBR);

    //──────────────────────────────────────────────
    // PARAMETER ATTACHMENTS
    //──────────────────────────────────────────────

    // Input
    inputGainAtt = std::make_unique<SliderAtt>(
        apvts, "inputGain",
        inputGainKnob.getSlider());

    // Saturation
    driveAtt = std::make_unique<SliderAtt>(
        apvts, "drive", driveKnob.getSlider());
    satMixAtt = std::make_unique<SliderAtt>(
        apvts, "satMix", satMixKnob.getSlider());
    satModelAtt = std::make_unique<ComboAtt>(
        apvts, "satModel", satModelSelector);
    satBypassAtt = std::make_unique<ButtonAtt>(
        apvts, "satBypass", satBypassBtn);

    // Compressor
    thresholdAtt = std::make_unique<SliderAtt>(
        apvts, "compThreshold",
        thresholdKnob.getSlider());
    ratioAtt = std::make_unique<SliderAtt>(
        apvts, "compRatio",
        ratioKnob.getSlider());
    attackAtt = std::make_unique<SliderAtt>(
        apvts, "compAttack",
        attackKnob.getSlider());
    releaseAtt = std::make_unique<SliderAtt>(
        apvts, "compRelease",
        releaseKnob.getSlider());
    makeupAtt = std::make_unique<SliderAtt>(
        apvts, "compMakeup",
        makeupKnob.getSlider());
    compMixAtt = std::make_unique<SliderAtt>(
        apvts, "compMix",
        compMixKnob.getSlider());
    kneeAtt = std::make_unique<SliderAtt>(
        apvts, "compKnee",
        kneeKnob.getSlider());
    compModelAtt = std::make_unique<ComboAtt>(
        apvts, "compModel", compModelSelector);
    compBypassAtt = std::make_unique<ButtonAtt>(
        apvts, "compBypass", compBypassBtn);
    fairchildTCAtt = std::make_unique<ComboAtt>(
        apvts, "fairchildTC", fairchildTCSelector);
    allInAtt = std::make_unique<ButtonAtt>(
        apvts, "allButtonsIn", allInBtn);
    thrustAtt = std::make_unique<ButtonAtt>(
        apvts, "thrustOn", thrustBtn);
    topologyAtt = std::make_unique<ButtonAtt>(
        apvts, "feedbackMode", topologyBtn);
    la2aLimitAtt = std::make_unique<ButtonAtt>(
        apvts, "la2aLimit", la2aLimitBtn);

    // EQ
    eqLowGainAtt = std::make_unique<SliderAtt>(
        apvts, "eqLowGain",
        eqLowGainKnob.getSlider());
    eqLowFreqAtt = std::make_unique<SliderAtt>(
        apvts, "eqLowFreq",
        eqLowFreqKnob.getSlider());
    eqMidGainAtt = std::make_unique<SliderAtt>(
        apvts, "eqMidGain",
        eqMidGainKnob.getSlider());
    eqMidFreqAtt = std::make_unique<SliderAtt>(
        apvts, "eqMidFreq",
        eqMidFreqKnob.getSlider());
    eqMidQAtt = std::make_unique<SliderAtt>(
        apvts, "eqMidQ",
        eqMidQKnob.getSlider());
    eqHighGainAtt = std::make_unique<SliderAtt>(
        apvts, "eqHighGain",
        eqHighGainKnob.getSlider());
    eqHighFreqAtt = std::make_unique<SliderAtt>(
        apvts, "eqHighFreq",
        eqHighFreqKnob.getSlider());
    eqHPFAtt = std::make_unique<SliderAtt>(
        apvts, "eqHPF",
        eqHPFKnob.getSlider());
    eqModelAtt = std::make_unique<ComboAtt>(
        apvts, "eqModel", eqModelSelector);
    eqBypassAtt = std::make_unique<ButtonAtt>(
        apvts, "eqBypass", eqBypassBtn);
    eqPreCompAtt = std::make_unique<ButtonAtt>(
        apvts, "eqPreComp", eqPreCompBtn);

    // Output
    outputGainAtt = std::make_unique<SliderAtt>(
        apvts, "outputGain",
        outputGainKnob.getSlider());

    // Listen to model selectors
    compModelSelector.addListener(this);
    eqModelSelector.addListener(this);

    // Set initial UI state
    updateCompressorUI(
        compModelSelector.getSelectedId() - 1);
    updateEQUI(
        eqModelSelector.getSelectedId() - 1);

    // Start meter timer at 30Hz
    startTimerHz(30);
}

ModulatedStripEditor::~ModulatedStripEditor()
{
    setLookAndFeel(nullptr);
    compModelSelector.removeListener(this);
    eqModelSelector.removeListener(this);
    stopTimer();
}

//==============================================================================
// COMBO LISTENER
//==============================================================================
void ModulatedStripEditor::comboBoxChanged(
    juce::ComboBox* box)
{
    if (box == &compModelSelector)
    {
        int idx = compModelSelector.getSelectedId() - 1;
        updateCompressorUI(idx);
        grNeedleMeter.setModel(idx);
    }

    if (box == &eqModelSelector)
        updateEQUI(
            eqModelSelector.getSelectedId() - 1);
}

//==============================================================================
// COMPRESSOR UI STATE MACHINE
//==============================================================================
void ModulatedStripEditor::updateCompressorUI(
    int modelIndex)
{
    // Hide all model-specific controls first
    fairchildTCSelector.setVisible(false);
    allInBtn    .setVisible(false);
    thrustBtn   .setVisible(false);
    topologyBtn .setVisible(false);
    la2aLimitBtn.setVisible(false);

    // Reset all knobs to active
    thresholdKnob.setModelState(true);
    ratioKnob    .setModelState(true);
    attackKnob   .setModelState(true);
    releaseKnob  .setModelState(true);
    makeupKnob   .setModelState(true);
    kneeKnob     .setModelState(true);

    thresholdKnob.setName("THRESH");
    ratioKnob    .setName("RATIO");
    attackKnob   .setName("ATTACK");
    releaseKnob  .setName("RELEASE");
    makeupKnob   .setName("MAKEUP");
    compModelHintLabel.setText("",
        juce::dontSendNotification);

    switch (modelIndex)
    {
        case 0: // SSL Bus
        {
            compModelHintLabel.setText(
                "VCA \xc2\xb7 Glue \xc2\xb7 Punchy",
                juce::dontSendNotification);
            kneeKnob.setName("KNEE");
            break;
        }

        case 1: // Fairchild 670
        {
            compModelHintLabel.setText(
                "Vari-Mu \xc2\xb7 Tube \xc2\xb7 Smooth",
                juce::dontSendNotification);

            attackKnob .setModelState(false,
                "TIME CONST");
            releaseKnob.setModelState(false,
                "TC SELECT");

            fairchildTCSelector.setVisible(true);

            ratioKnob.setModelState(false,
                "3:1 FIXED");
            kneeKnob.setName("KNEE (soft)");
            break;
        }

        case 2: // LA-2A
        {
            compModelHintLabel.setText(
                "Opto \xc2\xb7 T4B \xc2\xb7 Musical",
                juce::dontSendNotification);

            attackKnob .setModelState(false, "OPTICAL");
            releaseKnob.setModelState(false, "OPTICAL");
            ratioKnob  .setModelState(false,
                "COMP/LIMIT");
            kneeKnob   .setModelState(false, "OPTICAL");

            la2aLimitBtn.setVisible(true);

            thresholdKnob.setName("PEAK RED");
            makeupKnob   .setName("GAIN");
            break;
        }

        case 3: // 1176 FET
        {
            compModelHintLabel.setText(
                "FET \xc2\xb7 Fast \xc2\xb7 Aggressive",
                juce::dontSendNotification);

            thresholdKnob.setName("INPUT");
            attackKnob   .setName("ATTACK \xe2\x86\x90");
            kneeKnob     .setName("KNEE (FET)");

            allInBtn.setVisible(true);
            break;
        }

        case 4: // API 2500
        {
            compModelHintLabel.setText(
                "VCA \xc2\xb7 Thrust \xc2\xb7 Dense",
                juce::dontSendNotification);

            kneeKnob.setName("NEW/OLD");

            thrustBtn  .setVisible(true);
            topologyBtn.setVisible(true);
            break;
        }
    }

    repaint();
}

//==============================================================================
// EQ UI STATE MACHINE
//==============================================================================
void ModulatedStripEditor::updateEQUI(
    int modelIndex)
{
    // Reset all knobs
    eqLowGainKnob .setModelState(true);
    eqLowFreqKnob .setModelState(true);
    eqMidGainKnob .setModelState(true);
    eqMidFreqKnob .setModelState(true);
    eqMidQKnob    .setModelState(true);
    eqHighGainKnob.setModelState(true);
    eqHighFreqKnob.setModelState(true);
    eqHPFKnob     .setModelState(true);

    eqLowGainKnob .setName("LOW");
    eqLowFreqKnob .setName("LO FRQ");
    eqMidGainKnob .setName("MID");
    eqMidFreqKnob .setName("MI FRQ");
    eqMidQKnob    .setName("MID Q");
    eqHighGainKnob.setName("HIGH");
    eqHighFreqKnob.setName("HI FRQ");
    eqHPFKnob     .setName("HPF");
    eqModelHintLabel.setText("",
        juce::dontSendNotification);

    switch (modelIndex)
    {
        case 0: // Neve 1073
        {
            eqModelHintLabel.setText(
                "Transformer \xc2\xb7 Inductor \xc2\xb7 Bloom",
                juce::dontSendNotification);

            eqMidQKnob.setModelState(false, "PROP-Q");
            eqLowGainKnob.setName("LOW BOOST");
            eqHPFKnob.setName("HPF 6dB");
            break;
        }

        case 1: // Neve 1084
        {
            eqModelHintLabel.setText(
                "Transformer \xc2\xb7 Class A \xc2\xb7 Musical",
                juce::dontSendNotification);

            eqMidQKnob.setModelState(false, "PROP-Q");
            eqLowGainKnob.setName("LOW \xc2\xb1");
            eqHPFKnob.setName("HPF 6dB");
            break;
        }

        case 2: // SSL 4000E
        {
            eqModelHintLabel.setText(
                "SVF \xc2\xb7 Clean \xc2\xb7 Transparent",
                juce::dontSendNotification);

            eqMidQKnob.setModelState(false,
                "CONST Q");
            eqHPFKnob.setName("HPF 18dB");
            break;
        }

        case 3: // Pultec EQP-1A
        {
            eqModelHintLabel.setText(
                "Passive LC \xc2\xb7 Tube \xc2\xb7 Phase Bloom",
                juce::dontSendNotification);

            eqHPFKnob.setModelState(false, "NO HPF");
            eqLowGainKnob.setName("LO BOOST");
            eqMidQKnob.setModelState(false,
                "LC FIXED");
            break;
        }

        case 4: // API 550A
        {
            eqModelHintLabel.setText(
                "Discrete \xc2\xb7 Aggressive \xc2\xb7 Focused",
                juce::dontSendNotification);

            eqMidQKnob.setModelState(false, "PROP-Q");
            eqHPFKnob.setName("HPF 18dB");
            break;
        }
    }

    repaint();
}

//==============================================================================
// TIMER - Meters at 30Hz
//==============================================================================
void ModulatedStripEditor::timerCallback()
{
    inputMeter .setLevel(processor.getInputPeak());
    outputMeter.setLevel(processor.getOutputPeak());

    // GR needle meter
    float grDb = std::abs(processor.getGainReduction());
    grNeedleMeter.setGainReduction(grDb);
}

//==============================================================================
// PAINT - Backgrounds and panels
//==============================================================================
void ModulatedStripEditor::paint(juce::Graphics& g)
{
    // Main background
    g.fillAll(juce::Colour(0xFF0A0A0A));

    // Title bar background
    g.setColour(juce::Colour(0xFF111111));
    g.fillRect(0, 0, getWidth(), 42);

    // Title text - engraved look
    // Shadow
    g.setColour(juce::Colour(0xFF000000));
    g.setFont(juce::Font(
        juce::FontOptions(22.0f).withStyle("Bold")));
    g.drawText("MODULATED STRIP",
        1, 1, getWidth(), 42,
        juce::Justification::centred);
    // Main text
    g.setColour(juce::Colour(0xFFB89838));
    g.drawText("MODULATED STRIP",
        0, 0, getWidth(), 42,
        juce::Justification::centred);

    // Title underline
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawHorizontalLine(42, 0,
        static_cast<float>(getWidth()));

    // Section panels with brushed metal texture
    // INPUT
    PanelTextures::drawBrushedMetal(g,
        juce::Rectangle<float>(8, 48, 90, 544),
        juce::Colour(0xFF111111), 0.02f);

    // SATURATION - tint changes with model
    int satIdx = satModelSelector.getSelectedId() - 1;
    juce::Colour satPanelColor;
    switch (satIdx)
    {
        case 0:  satPanelColor = juce::Colour(0xFF111A11); break;
        case 1:  satPanelColor = juce::Colour(0xFF111111); break;
        case 2:  satPanelColor = juce::Colour(0xFF111118); break;
        case 3:  satPanelColor = juce::Colour(0xFF1A1511); break;
        case 4:  satPanelColor = juce::Colour(0xFF151210); break;
        case 5:  satPanelColor = juce::Colour(0xFF101215); break;
        case 6:  satPanelColor = juce::Colour(0xFF121212); break;
        default: satPanelColor = juce::Colour(0xFF111111); break;
    }
    PanelTextures::drawBrushedMetal(g,
        juce::Rectangle<float>(103, 48, 195, 544),
        satPanelColor, 0.025f);

    // COMPRESSOR - model specific panel color
    int compIdx = compModelSelector.getSelectedId() - 1;
    juce::Colour compPanelColor;
    switch (compIdx)
    {
        case 0:  compPanelColor = juce::Colour(0xFF1A1A1A); break;
        case 1:  compPanelColor = juce::Colour(0xFF1A1208); break;
        case 2:  compPanelColor = juce::Colour(0xFF1A1A18); break;
        case 3:  compPanelColor = juce::Colour(0xFF0F0F0F); break;
        case 4:  compPanelColor = juce::Colour(0xFF0F1520); break;
        default: compPanelColor = juce::Colour(0xFF111111); break;
    }
    PanelTextures::drawBrushedMetal(g,
        juce::Rectangle<float>(303, 48, 340, 544),
        compPanelColor, 0.02f);

    // EQ - model specific panel color
    int eqIdx = eqModelSelector.getSelectedId() - 1;
    juce::Colour eqPanelColor;
    switch (eqIdx)
    {
        case 0:  eqPanelColor = juce::Colour(0xFF0F1A10); break;
        case 1:  eqPanelColor = juce::Colour(0xFF0F1A10); break;
        case 2:  eqPanelColor = juce::Colour(0xFF1A1A1A); break;
        case 3:  eqPanelColor = juce::Colour(0xFF1A1810); break;
        case 4:  eqPanelColor = juce::Colour(0xFF0F0F1A); break;
        default: eqPanelColor = juce::Colour(0xFF111111); break;
    }
    PanelTextures::drawBrushedMetal(g,
        juce::Rectangle<float>(648, 48, 390, 544),
        eqPanelColor, 0.025f);

    // OUTPUT
    PanelTextures::drawBrushedMetal(g,
        juce::Rectangle<float>(1043, 48, 50, 544),
        juce::Colour(0xFF111111), 0.02f);

    // Section labels
    auto sf = juce::Font(
        juce::FontOptions(9.0f).withStyle("Bold"));
    g.setFont(sf);

    // Label shadow + text for each section
    auto drawSectionLabel = [&](float x, float w,
                                const juce::String& text)
    {
        g.setColour(juce::Colour(0xFF000000));
        g.drawText(text,
            static_cast<int>(x) + 1, 53,
            static_cast<int>(w), 14,
            juce::Justification::centred);
        g.setColour(juce::Colour(0xFFB89838));
        g.drawText(text,
            static_cast<int>(x), 52,
            static_cast<int>(w), 14,
            juce::Justification::centred);
    };

    drawSectionLabel(8,    90,  "INPUT");
    drawSectionLabel(103,  195, "SATURATION");
    drawSectionLabel(303,  340, "COMPRESSOR");
    drawSectionLabel(648,  390, "EQUALIZER");
    drawSectionLabel(1043, 50,  "OUT");

    // Dust overlay on all panels
    PanelTextures::drawDust(g,
        getLocalBounds().toFloat(), 0.01f);
}

//==============================================================================
// RESIZED - Position all controls
//==============================================================================
void ModulatedStripEditor::resized()
{
    int k  = 72;
    int lh = 14;

    // Screws
    screwTL.setBounds(12,  8, 14, 14);
    screwTR.setBounds(getWidth() - 26, 8, 14, 14);
    screwBL.setBounds(12, getHeight() - 22, 14, 14);
    screwBR.setBounds(getWidth() - 26,
                      getHeight() - 22, 14, 14);

    //──────────────────────────────────────────────
    // INPUT
    //──────────────────────────────────────────────
    inputGainKnob.setBounds(13, 70, k+lh, k+lh);
    inputMeterLabel.setBounds(13,
        70 + k + lh + 8, k + lh, 12);
    inputMeter.setBounds(25,
        70 + k + lh + 22, k - 20, 320);

    //──────────────────────────────────────────────
    // SATURATION
    //──────────────────────────────────────────────
    satModelSelector.setBounds(108, 70, 180, 24);
    satBypassBtn    .setBounds(248, 98, 40, 20);

    driveKnob .setBounds(108, 125, k+lh, k+lh);
    satMixKnob.setBounds(198, 125, k+lh, k+lh);

    //──────────────────────────────────────────────
    // COMPRESSOR
    //──────────────────────────────────────────────
    int cx = 308;

    compModelSelector.setBounds(cx, 70, 200, 24);
    compBypassBtn    .setBounds(cx + 210, 73, 40, 20);
    compModelHintLabel.setBounds(cx, 97, 250, 12);

    // VU Needle meter
    grNeedleMeter.setBounds(cx + 30, 112, 260, 120);

    // Model specific controls positioned below meter
    fairchildTCSelector.setBounds(cx, 238, 250, 24);
    allInBtn   .setBounds(cx + 220, 238, 60, 22);
    la2aLimitBtn.setBounds(cx + 220, 238, 60, 22);
    thrustBtn  .setBounds(cx + 170, 238, 60, 22);
    topologyBtn.setBounds(cx + 235, 238, 60, 22);

    // Row 1 - Threshold Ratio Knee
    int compKnobY = 268;
    thresholdKnob.setBounds(cx,       compKnobY,
        k+lh, k+lh);
    ratioKnob    .setBounds(cx + 105, compKnobY,
        k+lh, k+lh);
    kneeKnob     .setBounds(cx + 210, compKnobY,
        k+lh, k+lh);

    // Row 2 - Attack Release
    compKnobY += k + lh + 14;
    attackKnob .setBounds(cx,       compKnobY,
        k+lh, k+lh);
    releaseKnob.setBounds(cx + 105, compKnobY,
        k+lh, k+lh);

    // Row 3 - Makeup Mix
    compKnobY += k + lh + 14;
    makeupKnob .setBounds(cx,       compKnobY,
        k+lh, k+lh);
    compMixKnob.setBounds(cx + 105, compKnobY,
        k+lh, k+lh);

    //──────────────────────────────────────────────
    // EQ
    //──────────────────────────────────────────────
    int ex = 653;

    eqModelSelector.setBounds(ex, 70, 200, 24);
    eqBypassBtn    .setBounds(ex + 210, 73, 40, 20);
    eqPreCompBtn   .setBounds(ex + 260, 73, 60, 20);
    eqModelHintLabel.setBounds(ex, 97, 300, 12);

    // Low band
    int eqY = 115;
    eqLowGainKnob.setBounds(ex,       eqY, k+lh, k+lh);
    eqLowFreqKnob.setBounds(ex + 105, eqY, k+lh, k+lh);

    // Mid band
    eqY += k + lh + 14;
    eqMidGainKnob.setBounds(ex,       eqY, k+lh, k+lh);
    eqMidFreqKnob.setBounds(ex + 105, eqY, k+lh, k+lh);
    eqMidQKnob   .setBounds(ex + 210, eqY, k+lh, k+lh);

    // High band + HPF
    eqY += k + lh + 14;
    eqHighGainKnob.setBounds(ex,       eqY, k+lh, k+lh);
    eqHighFreqKnob.setBounds(ex + 105, eqY, k+lh, k+lh);
    eqHPFKnob     .setBounds(ex + 300, eqY, k+lh, k+lh);

    //──────────────────────────────────────────────
    // OUTPUT
    //──────────────────────────────────────────────
    outputGainKnob.setBounds(1045, 70, k+lh, k+lh);
    outputMeterLabel.setBounds(1045,
        70 + k + lh + 8, k + lh, 12);
    outputMeter.setBounds(1052,
        70 + k + lh + 22, 35, 320);
}