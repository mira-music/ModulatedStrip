#include "PluginEditor.h"

ModulatedStripEditor::ModulatedStripEditor(
    ModulatedStripProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(1100, 600);

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

    setupCombo(satModelSelector, {
        "NEVE", "SSL", "API", "TUBE",
        "TAPE", "FET", "IRON"});
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

    setupCombo(compModelSelector, {
        "SSL Bus", "Fairchild 670",
        "LA-2A", "1176", "API 2500"});
    addAndMakeVisible(compBypassBtn);

    // Model specific compressor controls
    setupCombo(fairchildTCSelector, {
        "TC 1  (0.2ms / 0.3s)",
        "TC 2  (0.2ms / 0.8s)",
        "TC 3  (0.4ms / 2.0s)",
        "TC 4  (0.4ms / Auto)",
        "TC 5  (0.4ms / 5.0s)",
        "TC 6  (0.4ms / Auto fast)"});
    fairchildTCSelector.setVisible(false);
    addAndMakeVisible(fairchildTCSelector);

    allInBtn   .setVisible(false);
    thrustBtn  .setVisible(false);
    topologyBtn.setVisible(false);
    la2aLimitBtn.setVisible(false);

    addAndMakeVisible(allInBtn);
    addAndMakeVisible(thrustBtn);
    addAndMakeVisible(topologyBtn);
    addAndMakeVisible(la2aLimitBtn);

    compModelHintLabel.setFont(juce::Font(
        juce::FontOptions(9.0f)));
    compModelHintLabel.setColour(
        juce::Label::textColourId,
        juce::Colour(0xFF888888));
    compModelHintLabel.setJustificationType(
        juce::Justification::centred);
    addAndMakeVisible(compModelHintLabel);

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

    setupCombo(eqModelSelector, {
        "Neve 1073", "Neve 1084", "SSL 4000E",
        "Pultec EQP-1A", "API 550A"});
    addAndMakeVisible(eqBypassBtn);
    addAndMakeVisible(eqPreCompBtn);

    eqModelHintLabel.setFont(juce::Font(
        juce::FontOptions(9.0f)));
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
    grMeter.setIsGainReduction(true);
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    addAndMakeVisible(grMeter);

    setupMeterLabel(inputMeterLabel,  "IN");
    setupMeterLabel(outputMeterLabel, "OUT");
    setupMeterLabel(grMeterLabel,     "GR");

    //──────────────────────────────────────────────
    // PARAMETER ATTACHMENTS
    //──────────────────────────────────────────────
    inputGainAtt = std::make_unique<SliderAtt>(
        apvts, "inputGain",
        inputGainKnob.getSlider());

    driveAtt = std::make_unique<SliderAtt>(
        apvts, "drive", driveKnob.getSlider());
    satMixAtt = std::make_unique<SliderAtt>(
        apvts, "satMix", satMixKnob.getSlider());
    satModelAtt = std::make_unique<ComboAtt>(
        apvts, "satModel", satModelSelector);
    satBypassAtt = std::make_unique<ButtonAtt>(
        apvts, "satBypass", satBypassBtn);

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

    outputGainAtt = std::make_unique<SliderAtt>(
        apvts, "outputGain",
        outputGainKnob.getSlider());

    // Listen to model selectors for UI updates
    compModelSelector.addListener(this);
    eqModelSelector.addListener(this);

    // Set initial UI state
    updateCompressorUI(compModelSelector.getSelectedId() - 1);
    updateEQUI(eqModelSelector.getSelectedId() - 1);

    startTimerHz(30);
}

ModulatedStripEditor::~ModulatedStripEditor()
{
    compModelSelector.removeListener(this);
    eqModelSelector.removeListener(this);
    stopTimer();
}

//==============================================================================
// COMBO BOX LISTENER
// Called when user changes model selector
//==============================================================================
void ModulatedStripEditor::comboBoxChanged(
    juce::ComboBox* box)
{
    if (box == &compModelSelector)
        updateCompressorUI(
            compModelSelector.getSelectedId() - 1);

    if (box == &eqModelSelector)
        updateEQUI(
            eqModelSelector.getSelectedId() - 1);
}

//==============================================================================
// COMPRESSOR UI STATE MACHINE
// Each model reconfigures the controls
//==============================================================================
void ModulatedStripEditor::updateCompressorUI(
    int modelIndex)
{
    // Reset all model-specific controls first
    fairchildTCSelector.setVisible(false);
    allInBtn           .setVisible(false);
    thrustBtn          .setVisible(false);
    topologyBtn        .setVisible(false);
    la2aLimitBtn       .setVisible(false);

    // Reset all knobs to active state
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
            // Authentic SSL character info
            compModelHintLabel.setText(
                "VCA · Glue · Punchy",
                juce::dontSendNotification);

            // Knee defaults hard on SSL
            kneeKnob.setName("KNEE");
            break;
        }

        case 1: // Fairchild 670
        {
            compModelHintLabel.setText(
                "Vari-Mu · Tube · Smooth",
                juce::dontSendNotification);

            // Attack and release replaced by
            // 6-position time constant selector
            attackKnob .setModelState(false,
                "TIME CONST");
            releaseKnob.setModelState(false,
                "→ TC SELECT");

            // Show time constant selector
            fairchildTCSelector.setVisible(true);

            // Ratio is fixed at ~3:1 for vari-mu
            ratioKnob.setModelState(false, "3:1 FIXED");

            // Very soft knee - tube physics
            kneeKnob.setName("KNEE (soft)");
            break;
        }

        case 2: // LA-2A
        {
            compModelHintLabel.setText(
                "Opto · T4B · Musical",
                juce::dontSendNotification);

            // Attack is fixed by opto physics
            attackKnob.setModelState(false, "OPTICAL");

            // Ratio replaced by compress/limit toggle
            ratioKnob.setModelState(false,
                "COMP/LIMIT");
            la2aLimitBtn.setVisible(true);

            // Threshold renamed Peak Reduction
            thresholdKnob.setName("PEAK RED");

            // Makeup renamed Gain
            makeupKnob.setName("GAIN");

            // Knee handled by opto physics
            kneeKnob.setModelState(false, "OPTICAL");
            break;
        }

        case 3: // 1176 FET
        {
            compModelHintLabel.setText(
                "FET · Fast · Aggressive",
                juce::dontSendNotification);

            // Threshold renamed Input
            thresholdKnob.setName("INPUT");

            // Attack note - runs backwards
            attackKnob.setName("ATTACK←");

            // All-buttons-in mode
            allInBtn.setVisible(true);

            // Tight FET knee
            kneeKnob.setName("KNEE (FET)");
            break;
        }

        case 4: // API 2500
        {
            compModelHintLabel.setText(
                "VCA · Thrust · Dense",
                juce::dontSendNotification);

            // API specific controls
            thrustBtn  .setVisible(true);
            topologyBtn.setVisible(true);

            // New/Old knee modes
            kneeKnob.setName("NEW/OLD");
            break;
        }
    }

    resized();
    repaint();
}

//==============================================================================
// EQ UI STATE MACHINE
// Each model reconfigures the EQ controls
//==============================================================================
void ModulatedStripEditor::updateEQUI(int modelIndex)
{
    // Reset all EQ knobs to active
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
                "Transformer · Inductor · Bloom",
                juce::dontSendNotification);

            // Proportional Q - user cannot control
            eqMidQKnob.setModelState(false,
                "PROP-Q");

            // Low shelf is boost only
            eqLowGainKnob.setName("LOW BOOST");

            // HPF is 6dB/oct - label it
            eqHPFKnob.setName("HPF 6dB");
            break;
        }

        case 1: // Neve 1084
        {
            eqModelHintLabel.setText(
                "Transformer · Class A · Musical",
                juce::dontSendNotification);

            // Proportional Q but user can add to it
            eqMidQKnob.setModelState(false,
                "PROP-Q");

            // 1084 allows low shelf cut
            eqLowGainKnob.setName("LOW ±");

            eqHPFKnob.setName("HPF 6dB");
            break;
        }

        case 2: // SSL 4000E
        {
            eqModelHintLabel.setText(
                "SVF · Clean · Transparent",
                juce::dontSendNotification);

            // Constant Q - Q knob does nothing on SSL
            eqMidQKnob.setModelState(false,
                "CONST Q");

            // HPF is 18dB/oct on SSL
            eqHPFKnob.setName("HPF 18dB");
            break;
        }

        case 3: // Pultec EQP-1A
        {
            eqModelHintLabel.setText(
                "Passive LC · Tube · Phase Bloom",
                juce::dontSendNotification);

            // Pultec has no HPF
            eqHPFKnob.setModelState(false, "NO HPF");

            // Low is boost only on Pultec
            eqLowGainKnob.setName("LO BOOST");

            // Q is fixed by LC network
            eqMidQKnob.setModelState(false,
                "LC FIXED");
            break;
        }

        case 4: // API 550A
        {
            eqModelHintLabel.setText(
                "Discrete · Aggressive · Focused",
                juce::dontSendNotification);

            // Proportional Q - quadratic growth
            eqMidQKnob.setModelState(false,
                "PROP-Q");

            // HPF is 18dB/oct on API
            eqHPFKnob.setName("HPF 18dB");
            break;
        }
    }

    resized();
    repaint();
}

//==============================================================================
// TIMER - Meters at 30Hz
//==============================================================================

void ModulatedStripEditor::timerCallback()
{
    inputMeter .setLevel(processor.getInputPeak());
    outputMeter.setLevel(processor.getOutputPeak());

    // GR meter correct scaling
    // 0dB GR  = grNorm 0.0 = bar empty
    // 20dB GR = grNorm 1.0 = bar full
    float grDb   = std::abs(processor.getGainReduction());
    float grNorm = juce::jlimit(0.0f, 1.0f, grDb / 20.0f);
    float grLevel = juce::Decibels::decibelsToGain(
        -60.0f * (1.0f - grNorm));
    grMeter.setLevel(grLevel);
}

//==============================================================================
// SETUP HELPERS
//==============================================================================
void ModulatedStripEditor::setupCombo(
    SteppedCombo& box,
    const juce::StringArray& items)
{
    box.addItemList(items, 1);
    addAndMakeVisible(box);
}

void ModulatedStripEditor::setupMeterLabel(
    juce::Label& label,
    const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId,
        juce::Colour(0xFF888888));
    label.setFont(juce::Font(juce::FontOptions(9.0f)));
    label.setJustificationType(
        juce::Justification::centred);
    addAndMakeVisible(label);
}

//==============================================================================
// PAINT
//==============================================================================
void ModulatedStripEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF0A0A0A));

    // Title
    g.setColour(juce::Colour(0xFFE8A838));
    g.setFont(juce::Font(
        juce::FontOptions(24.0f).withStyle("Bold")));
    g.drawText("MODULATED STRIP",
        0, 0, getWidth(), 42,
        juce::Justification::centred);

    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawHorizontalLine(42, 15.0f,
        getWidth() - 15.0f);

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

    drawPanel(  8,  48,  90, 540, "INPUT");
    drawPanel(103,  48, 195, 540, "SATURATION");
    drawPanel(303,  48, 310, 540, "COMPRESSOR");
    drawPanel(618,  48, 415, 540, "EQUALIZER");
    drawPanel(1038, 48,  55, 540, "OUT");
}

//==============================================================================
// RESIZED - Position everything
//==============================================================================
void ModulatedStripEditor::resized()
{
    int k  = 72;   // knob component height
    int lh = 14;   // label height inside ModelKnob

    //──────────────────────────────────────────────
    // INPUT
    //──────────────────────────────────────────────
    inputGainKnob .setBounds(13, 68,  k+lh, k+lh);
    inputMeterLabel.setBounds(13, 68 + k+lh + 5, k+lh, lh);
    inputMeter     .setBounds(20, 68 + k+lh + 5 + lh,
        k - 10, 280);

    //──────────────────────────────────────────────
    // SATURATION
    //──────────────────────────────────────────────
    satModelSelector.setBounds(108, 68, 180, 24);
    satBypassBtn    .setBounds(293, 71,  35, 20);

    driveKnob .setBounds(108, 100, k+lh, k+lh);
    satMixKnob.setBounds(198, 100, k+lh, k+lh);

    //──────────────────────────────────────────────
    // COMPRESSOR
    //──────────────────────────────────────────────
    int cx = 308;
    int cy = 68;

    compModelSelector.setBounds(cx, cy,      190, 24);
    compBypassBtn    .setBounds(cx+198, cy+4, 35, 20);
    compModelHintLabel.setBounds(cx, cy+28, 240, 14);

    // Time constant selector (Fairchild only)
    fairchildTCSelector.setBounds(cx, cy+48, 240, 24);

    // Extra buttons - positioned in compressor section
    allInBtn   .setBounds(cx+210, cy+48, 50, 22);
    la2aLimitBtn.setBounds(cx+210, cy+48, 55, 22);
    thrustBtn  .setBounds(cx+210, cy+48, 50, 22);
    topologyBtn.setBounds(cx+210, cy+75, 50, 22);

    cy += 50;

    // Row 1 - Threshold Ratio Knee
    thresholdKnob.setBounds(cx,       cy, k+lh, k+lh);
    ratioKnob    .setBounds(cx + 105, cy, k+lh, k+lh);
    kneeKnob     .setBounds(cx + 210, cy, k+lh, k+lh);

    cy += k + lh + 18;

    // Row 2 - Attack Release
    attackKnob .setBounds(cx,       cy, k+lh, k+lh);
    releaseKnob.setBounds(cx + 105, cy, k+lh, k+lh);

    cy += k + lh + 18;

    // Row 3 - Makeup Mix
    makeupKnob .setBounds(cx,       cy, k+lh, k+lh);
    compMixKnob.setBounds(cx + 105, cy, k+lh, k+lh);

    // GR meter
    grMeterLabel.setBounds(cx + 230, cy, 50, lh);
    grMeter     .setBounds(cx + 242, cy + lh, 20, k + 40);

    //──────────────────────────────────────────────
    // EQ
    //──────────────────────────────────────────────
    int ex = 623;
    int ey = 68;

    eqModelSelector.setBounds(ex, ey,       195, 24);
    eqBypassBtn    .setBounds(ex+200, ey+4,  35, 20);
    eqPreCompBtn   .setBounds(ex+240, ey+4,  55, 20);
    eqModelHintLabel.setBounds(ex, ey+28,   300, 14);

    ey += 50;

    // Low band
    eqLowGainKnob.setBounds(ex,       ey, k+lh, k+lh);
    eqLowFreqKnob.setBounds(ex + 105, ey, k+lh, k+lh);

    ey += k + lh + 18;

    // Mid band
    eqMidGainKnob.setBounds(ex,       ey, k+lh, k+lh);
    eqMidFreqKnob.setBounds(ex + 105, ey, k+lh, k+lh);
    eqMidQKnob   .setBounds(ex + 210, ey, k+lh, k+lh);

    ey += k + lh + 18;

    // High band and HPF
    eqHighGainKnob.setBounds(ex,       ey, k+lh, k+lh);
    eqHighFreqKnob.setBounds(ex + 105, ey, k+lh, k+lh);
    eqHPFKnob     .setBounds(ex + 300, ey, k+lh, k+lh);

    //──────────────────────────────────────────────
    // OUTPUT
    //──────────────────────────────────────────────
    outputGainKnob .setBounds(1040, 68, k+lh, k+lh);
    outputMeterLabel.setBounds(1040,
        68 + k+lh + 5, k+lh, lh);
    outputMeter    .setBounds(1047,
        68 + k+lh + 5 + lh, k - 10, 280);
}