#include "PluginEditor.h"

ModulatedStripEditor::ModulatedStripEditor(
    ModulatedStripProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      presetBar(p.presetManager)
{
    setSize(1280, 640);
    setLookAndFeel(&analogLAF);

    auto& apvts = processor.apvts;

    //──────────────────────────────────────────────
    // PRESET BAR
    //──────────────────────────────────────────────
    addAndMakeVisible(presetBar);
    presetBar.onOpenBrowser = [this] { toggleBrowser(); };
    presetBar.onPresetChanged = [this] {
        compModelSelector.setSelectedId(
            static_cast<int>(processor.apvts
                .getRawParameterValue("compModel")
                ->load()) + 1,
            juce::dontSendNotification);
        eqModelSelector.setSelectedId(
            static_cast<int>(processor.apvts
                .getRawParameterValue("eqModel")
                ->load()) + 1,
            juce::dontSendNotification);
        satModelSelector.setSelectedId(
            static_cast<int>(processor.apvts
                .getRawParameterValue("satModel")
                ->load()) + 1,
            juce::dontSendNotification);
        currentCompModel = compModelSelector.getSelectedId() - 1;
        currentEQModel   = eqModelSelector.getSelectedId() - 1;
        currentSatModel  = satModelSelector.getSelectedId() - 1;
        updateCompressorUI(currentCompModel);
        updateEQUI(currentEQModel);
        grNeedleMeter.setModel(currentCompModel);
        repaint();
    };

    //──────────────────────────────────────────────
    // INPUT
    //──────────────────────────────────────────────
    inputGainKnob.setName("INPUT");
    setKnobVisualStyle(inputGainKnob.getSlider(), KnobVisualStyle::DarkMetal);
    addAndMakeVisible(inputGainKnob);

    //──────────────────────────────────────────────
    // SATURATION
    //──────────────────────────────────────────────
    driveKnob.setName("DRIVE");
    satMixKnob.setName("SAT MIX");
    setKnobVisualStyle(driveKnob.getSlider(), KnobVisualStyle::DarkMetal);
    setKnobVisualStyle(satMixKnob.getSlider(), KnobVisualStyle::DarkMetal);
    addAndMakeVisible(driveKnob);
    addAndMakeVisible(satMixKnob);
    satModelSelector.addItemList({"NEVE","SSL","API","TUBE","TAPE","FET","IRON"}, 1);
    addAndMakeVisible(satModelSelector);
    satBypassBtn.getProperties().set("isBypass", true);
    addAndMakeVisible(satBypassBtn);

    //──────────────────────────────────────────────
    // COMPRESSOR
    //──────────────────────────────────────────────
    thresholdKnob.setName("THRESH");
    ratioKnob.setName("RATIO");
    attackKnob.setName("ATTACK");
    releaseKnob.setName("RELEASE");
    makeupKnob.setName("MAKEUP");
    compMixKnob.setName("MIX");
    kneeKnob.setName("KNEE");
    addAndMakeVisible(thresholdKnob);
    addAndMakeVisible(ratioKnob);
    addAndMakeVisible(attackKnob);
    addAndMakeVisible(releaseKnob);
    addAndMakeVisible(makeupKnob);
    addAndMakeVisible(compMixKnob);
    addAndMakeVisible(kneeKnob);
    compModelSelector.addItemList({"SSL Bus","Fairchild 670","LA-2A","1176","API 2500"}, 1);
    addAndMakeVisible(compModelSelector);
    compBypassBtn.getProperties().set("isBypass", true);
    addAndMakeVisible(compBypassBtn);
    fairchildTCSelector.addItemList({
        "TC 1 (0.2ms/0.3s)","TC 2 (0.2ms/0.8s)","TC 3 (0.4ms/2.0s)",
        "TC 4 (0.4ms/Auto)","TC 5 (0.4ms/5.0s)","TC 6 (0.4ms/Auto)"}, 1);
    fairchildTCSelector.setVisible(false);
    addAndMakeVisible(fairchildTCSelector);
    allInBtn.setVisible(false);
    thrustBtn.setVisible(false);
    topologyBtn.setVisible(false);
    la2aLimitBtn.setVisible(false);
    addAndMakeVisible(allInBtn);
    addAndMakeVisible(thrustBtn);
    addAndMakeVisible(topologyBtn);
    addAndMakeVisible(la2aLimitBtn);
    compModelHintLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    compModelHintLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF666666));
    compModelHintLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(compModelHintLabel);
    addAndMakeVisible(grNeedleMeter);

    //──────────────────────────────────────────────
    // EQ
    //──────────────────────────────────────────────
    eqLowGainKnob.setName("LOW");
    eqLowFreqKnob.setName("LO FRQ");
    eqMidGainKnob.setName("MID");
    eqMidFreqKnob.setName("MI FRQ");
    eqMidQKnob.setName("MID Q");
    eqHighGainKnob.setName("HIGH");
    eqHighFreqKnob.setName("HI FRQ");
    eqHPFKnob.setName("HPF");
    addAndMakeVisible(eqLowGainKnob);
    addAndMakeVisible(eqLowFreqKnob);
    addAndMakeVisible(eqMidGainKnob);
    addAndMakeVisible(eqMidFreqKnob);
    addAndMakeVisible(eqMidQKnob);
    addAndMakeVisible(eqHighGainKnob);
    addAndMakeVisible(eqHighFreqKnob);
    addAndMakeVisible(eqHPFKnob);
    eqModelSelector.addItemList({"Neve 1073","Neve 1084","SSL 4000E","Pultec EQP-1A","API 550A"}, 1);
    addAndMakeVisible(eqModelSelector);
    eqBypassBtn.getProperties().set("isBypass", true);
    addAndMakeVisible(eqBypassBtn);
    addAndMakeVisible(eqPreCompBtn);
    eqModelHintLabel.setFont(juce::Font(juce::FontOptions(7.5f)));
    eqModelHintLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF666666));
    eqModelHintLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(eqModelHintLabel);

    //──────────────────────────────────────────────
    // OUTPUT
    //──────────────────────────────────────────────
    outputGainKnob.setName("OUTPUT");
    setKnobVisualStyle(outputGainKnob.getSlider(), KnobVisualStyle::DarkMetal);
    addAndMakeVisible(outputGainKnob);

    //──────────────────────────────────────────────
    // METERS
    //──────────────────────────────────────────────
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    auto setupLabel = [this](juce::Label& l, const juce::String& t) {
        l.setText(t, juce::dontSendNotification);
        l.setColour(juce::Label::textColourId, juce::Colour(0xFF666666));
        l.setFont(juce::Font(juce::FontOptions(7.0f).withStyle("Bold")));
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(l);
    };
    setupLabel(inputMeterLabel, "IN");
    setupLabel(outputMeterLabel, "OUT");

    //──────────────────────────────────────────────
    // NEW FEATURE CONTROLS
    //──────────────────────────────────────────────
    oversampleSelector.addItemList({"1x (Off)","2x","4x","8x"}, 1);
    addAndMakeVisible(oversampleSelector);
    addAndMakeVisible(deltaBtn);
    addAndMakeVisible(analogBypassBtn);
    addAndMakeVisible(crosstalkBtn);
    addAndMakeVisible(noiseFloorBtn);
    stereoModeSelector.addItemList({"Stereo","Mid/Side","Dual Mono"}, 1);
    addAndMakeVisible(stereoModeSelector);

    //──────────────────────────────────────────────
    // ALL PARAMETER ATTACHMENTS
    //──────────────────────────────────────────────
    inputGainAtt = std::make_unique<SliderAtt>(apvts, "inputGain", inputGainKnob.getSlider());
    driveAtt     = std::make_unique<SliderAtt>(apvts, "drive", driveKnob.getSlider());
    satMixAtt    = std::make_unique<SliderAtt>(apvts, "satMix", satMixKnob.getSlider());
    satModelAtt  = std::make_unique<ComboAtt>(apvts, "satModel", satModelSelector);
    satBypassAtt = std::make_unique<ButtonAtt>(apvts, "satBypass", satBypassBtn);

    thresholdAtt = std::make_unique<SliderAtt>(apvts, "compThreshold", thresholdKnob.getSlider());
    ratioAtt     = std::make_unique<SliderAtt>(apvts, "compRatio", ratioKnob.getSlider());
    attackAtt    = std::make_unique<SliderAtt>(apvts, "compAttack", attackKnob.getSlider());
    releaseAtt   = std::make_unique<SliderAtt>(apvts, "compRelease", releaseKnob.getSlider());
    makeupAtt    = std::make_unique<SliderAtt>(apvts, "compMakeup", makeupKnob.getSlider());
    compMixAtt   = std::make_unique<SliderAtt>(apvts, "compMix", compMixKnob.getSlider());
    kneeAtt      = std::make_unique<SliderAtt>(apvts, "compKnee", kneeKnob.getSlider());
    compModelAtt = std::make_unique<ComboAtt>(apvts, "compModel", compModelSelector);
    compBypassAtt = std::make_unique<ButtonAtt>(apvts, "compBypass", compBypassBtn);
    fairchildTCAtt = std::make_unique<ComboAtt>(apvts, "fairchildTC", fairchildTCSelector);
    allInAtt     = std::make_unique<ButtonAtt>(apvts, "allButtonsIn", allInBtn);
    thrustAtt    = std::make_unique<ButtonAtt>(apvts, "thrustOn", thrustBtn);
    topologyAtt  = std::make_unique<ButtonAtt>(apvts, "feedbackMode", topologyBtn);
    la2aLimitAtt = std::make_unique<ButtonAtt>(apvts, "la2aLimit", la2aLimitBtn);

    eqLowGainAtt  = std::make_unique<SliderAtt>(apvts, "eqLowGain", eqLowGainKnob.getSlider());
    eqLowFreqAtt  = std::make_unique<SliderAtt>(apvts, "eqLowFreq", eqLowFreqKnob.getSlider());
    eqMidGainAtt  = std::make_unique<SliderAtt>(apvts, "eqMidGain", eqMidGainKnob.getSlider());
    eqMidFreqAtt  = std::make_unique<SliderAtt>(apvts, "eqMidFreq", eqMidFreqKnob.getSlider());
    eqMidQAtt     = std::make_unique<SliderAtt>(apvts, "eqMidQ", eqMidQKnob.getSlider());
    eqHighGainAtt = std::make_unique<SliderAtt>(apvts, "eqHighGain", eqHighGainKnob.getSlider());
    eqHighFreqAtt = std::make_unique<SliderAtt>(apvts, "eqHighFreq", eqHighFreqKnob.getSlider());
    eqHPFAtt      = std::make_unique<SliderAtt>(apvts, "eqHPF", eqHPFKnob.getSlider());
    eqModelAtt    = std::make_unique<ComboAtt>(apvts, "eqModel", eqModelSelector);
    eqBypassAtt   = std::make_unique<ButtonAtt>(apvts, "eqBypass", eqBypassBtn);
    eqPreCompAtt  = std::make_unique<ButtonAtt>(apvts, "eqPreComp", eqPreCompBtn);

    outputGainAtt = std::make_unique<SliderAtt>(apvts, "outputGain", outputGainKnob.getSlider());

    oversampleAtt   = std::make_unique<ComboAtt>(apvts, "oversample", oversampleSelector);
    deltaAtt        = std::make_unique<ButtonAtt>(apvts, "delta", deltaBtn);
    analogBypassAtt = std::make_unique<ButtonAtt>(apvts, "analogBypass", analogBypassBtn);
    stereoModeAtt   = std::make_unique<ComboAtt>(apvts, "stereoMode", stereoModeSelector);

    crosstalkAtt  = std::make_unique<ButtonAtt>(apvts, "crosstalk", crosstalkBtn);
    noiseFloorAtt = std::make_unique<ButtonAtt>(apvts, "noiseFloor", noiseFloorBtn);

    //──────────────────────────────────────────────
    // LISTENERS
    //──────────────────────────────────────────────
    compModelSelector.addListener(this);
    eqModelSelector.addListener(this);
    satModelSelector.addListener(this);

    updateCompressorUI(compModelSelector.getSelectedId() - 1);
    updateEQUI(eqModelSelector.getSelectedId() - 1);

    startTimerHz(30);
}

ModulatedStripEditor::~ModulatedStripEditor()
{
    presetBrowser.reset();
    setLookAndFeel(nullptr);
    compModelSelector.removeListener(this);
    eqModelSelector.removeListener(this);
    satModelSelector.removeListener(this);
    stopTimer();
}

//==============================================================================
void ModulatedStripEditor::toggleBrowser()
{
    if (presetBrowser == nullptr)
    {
        presetBrowser = std::make_unique<PresetBrowser>(processor.presetManager);
        presetBrowser->onPresetLoaded = [this] {
            presetBar.update();
            if (presetBar.onPresetChanged)
                presetBar.onPresetChanged();
        };
        addAndMakeVisible(*presetBrowser);
        presetBrowser->setBounds(
            getWidth()/2 - 210, getHeight()/2 - 260, 420, 520);
    }
    bool show = !presetBrowser->isVisible();
    presetBrowser->setVisible(show);
    if (show)
    {
        presetBrowser->refresh();
        presetBrowser->toFront(true);
    }
}

//==============================================================================
juce::Colour ModulatedStripEditor::getCompPanelColor(int idx)
{
    switch (idx)
    {
        case 0: return juce::Colour(0xFF1A1A1C);
        case 1: return juce::Colour(0xFF1C1410);
        case 2: return juce::Colour(0xFF1A1A18);
        case 3: return juce::Colour(0xFF0F0F0F);
        case 4: return juce::Colour(0xFF0F1520);
        default: return juce::Colour(0xFF141414);
    }
}

juce::Colour ModulatedStripEditor::getEQPanelColor(int idx)
{
    switch (idx)
    {
        case 0: return juce::Colour(0xFF0F1A10);
        case 1: return juce::Colour(0xFF101A10);
        case 2: return juce::Colour(0xFF1A1A1C);
        case 3: return juce::Colour(0xFF1A1810);
        case 4: return juce::Colour(0xFF0F0F1A);
        default: return juce::Colour(0xFF141414);
    }
}

juce::Colour ModulatedStripEditor::getSatPanelColor(int idx)
{
    switch (idx)
    {
        case 0: return juce::Colour(0xFF111A11);
        case 1: return juce::Colour(0xFF111111);
        case 2: return juce::Colour(0xFF111118);
        case 3: return juce::Colour(0xFF1A1511);
        case 4: return juce::Colour(0xFF151210);
        case 5: return juce::Colour(0xFF101215);
        case 6: return juce::Colour(0xFF121212);
        default: return juce::Colour(0xFF111111);
    }
}

//==============================================================================
void ModulatedStripEditor::comboBoxChanged(juce::ComboBox* box)
{
    if (box == &compModelSelector)
    {
        currentCompModel = compModelSelector.getSelectedId() - 1;
        updateCompressorUI(currentCompModel);
        grNeedleMeter.setModel(currentCompModel);
    }
    if (box == &eqModelSelector)
    {
        currentEQModel = eqModelSelector.getSelectedId() - 1;
        updateEQUI(currentEQModel);
    }
    if (box == &satModelSelector)
        currentSatModel = satModelSelector.getSelectedId() - 1;
    repaint();
}

//==============================================================================
void ModulatedStripEditor::updateCompressorUI(int mi)
{
    fairchildTCSelector.setVisible(false);
    allInBtn.setVisible(false);
    thrustBtn.setVisible(false);
    topologyBtn.setVisible(false);
    la2aLimitBtn.setVisible(false);

    thresholdKnob.setModelState(true);
    ratioKnob.setModelState(true);
    attackKnob.setModelState(true);
    releaseKnob.setModelState(true);
    makeupKnob.setModelState(true);
    kneeKnob.setModelState(true);

    thresholdKnob.setName("THRESH");
    ratioKnob.setName("RATIO");
    attackKnob.setName("ATTACK");
    releaseKnob.setName("RELEASE");
    makeupKnob.setName("MAKEUP");
    kneeKnob.setName("KNEE");
    compModelHintLabel.setText("", juce::dontSendNotification);

    auto applyStyle = [&](KnobVisualStyle s) {
        setKnobVisualStyle(thresholdKnob.getSlider(), s);
        setKnobVisualStyle(ratioKnob.getSlider(), s);
        setKnobVisualStyle(attackKnob.getSlider(), s);
        setKnobVisualStyle(releaseKnob.getSlider(), s);
        setKnobVisualStyle(makeupKnob.getSlider(), s);
        setKnobVisualStyle(compMixKnob.getSlider(), s);
        setKnobVisualStyle(kneeKnob.getSlider(), s);
    };

    switch (mi)
    {
        case 0:
            applyStyle(KnobVisualStyle::SSLCompact);
            compModelHintLabel.setText("VCA | Glue | Punchy", juce::dontSendNotification);
            break;
        case 1:
            applyStyle(KnobVisualStyle::VintageDome);
            compModelHintLabel.setText("Vari-Mu | Tube | Smooth", juce::dontSendNotification);
            attackKnob.setModelState(false, "TIME CONST");
            releaseKnob.setModelState(false, "TC SELECT");
            ratioKnob.setModelState(false, "VARI-MU");
            kneeKnob.setName("BIAS");
            fairchildTCSelector.setVisible(true);
            break;
        case 2:
            applyStyle(KnobVisualStyle::VintageDome);
            compModelHintLabel.setText("Opto | T4B | Musical", juce::dontSendNotification);
            attackKnob.setModelState(false, "OPTICAL");
            releaseKnob.setModelState(false, "OPTICAL");
            ratioKnob.setModelState(false, "COMP/LIM");
            kneeKnob.setModelState(false, "OPTICAL");
            thresholdKnob.setName("PEAK RED");
            makeupKnob.setName("GAIN");
            la2aLimitBtn.setVisible(true);
            break;
        case 3:
            applyStyle(KnobVisualStyle::SSLCompact);
            compModelHintLabel.setText("FET | Fast | Aggressive", juce::dontSendNotification);
            thresholdKnob.setName("INPUT");
            makeupKnob.setName("OUTPUT");
            attackKnob.setName("ATK \xe2\x86\x90");
            kneeKnob.setModelState(false, "HARD");
            allInBtn.setVisible(true);
            break;
        case 4:
            applyStyle(KnobVisualStyle::APISkirt);
            compModelHintLabel.setText("VCA | Thrust | Dense", juce::dontSendNotification);
            kneeKnob.setName("NEW/OLD");
            thrustBtn.setVisible(true);
            topologyBtn.setVisible(true);
            break;
    }
    repaint();
}

//==============================================================================
void ModulatedStripEditor::updateEQUI(int mi)
{
    eqLowGainKnob.setModelState(true);
    eqLowFreqKnob.setModelState(true);
    eqMidGainKnob.setModelState(true);
    eqMidFreqKnob.setModelState(true);
    eqMidQKnob.setModelState(true);
    eqHighGainKnob.setModelState(true);
    eqHighFreqKnob.setModelState(true);
    eqHPFKnob.setModelState(true);

    eqLowGainKnob.setName("LOW");
    eqLowFreqKnob.setName("LO FRQ");
    eqMidGainKnob.setName("MID");
    eqMidFreqKnob.setName("MI FRQ");
    eqMidQKnob.setName("MID Q");
    eqHighGainKnob.setName("HIGH");
    eqHighFreqKnob.setName("HI FRQ");
    eqHPFKnob.setName("HPF");
    eqModelHintLabel.setText("", juce::dontSendNotification);

    auto applyStyle = [&](KnobVisualStyle s) {
        setKnobVisualStyle(eqLowGainKnob.getSlider(), s);
        setKnobVisualStyle(eqLowFreqKnob.getSlider(), s);
        setKnobVisualStyle(eqMidGainKnob.getSlider(), s);
        setKnobVisualStyle(eqMidFreqKnob.getSlider(), s);
        setKnobVisualStyle(eqMidQKnob.getSlider(), s);
        setKnobVisualStyle(eqHighGainKnob.getSlider(), s);
        setKnobVisualStyle(eqHighFreqKnob.getSlider(), s);
        setKnobVisualStyle(eqHPFKnob.getSlider(), s);
    };

    switch (mi)
    {
        case 0:
            applyStyle(KnobVisualStyle::DarkMetal);
            eqModelHintLabel.setText("Transformer | Inductor | Bloom", juce::dontSendNotification);
            eqMidQKnob.setModelState(false, "PROP-Q");
            eqLowGainKnob.setName("LF BOOST");
            eqHPFKnob.setName("HPF 6dB");
            break;
        case 1:
            applyStyle(KnobVisualStyle::DarkMetal);
            eqModelHintLabel.setText("Transformer | Class A | Musical", juce::dontSendNotification);
            eqMidQKnob.setModelState(false, "PROP-Q");
            eqHPFKnob.setName("HPF 6dB");
            break;
        case 2:
            applyStyle(KnobVisualStyle::SSLCompact);
            eqModelHintLabel.setText("SVF | Clean | Transparent", juce::dontSendNotification);
            eqMidQKnob.setModelState(false, "CONST Q");
            eqHPFKnob.setName("HPF 18dB");
            break;
        case 3:
            applyStyle(KnobVisualStyle::VintageDome);
            eqModelHintLabel.setText("Passive LC | Tube | Phase Bloom", juce::dontSendNotification);
            eqHPFKnob.setModelState(false, "NO HPF");
            eqLowGainKnob.setName("LF BOOST");
            eqMidQKnob.setModelState(false, "LC FIXED");
            eqMidGainKnob.setName("HF BOOST");
            eqMidFreqKnob.setName("HF FRQ");
            break;
        case 4:
            applyStyle(KnobVisualStyle::APISkirt);
            eqModelHintLabel.setText("Discrete | Aggressive | Focused", juce::dontSendNotification);
            eqMidQKnob.setModelState(false, "PROP-Q");
            eqHPFKnob.setName("HPF 18dB");
            break;
    }
    repaint();
}

//==============================================================================
void ModulatedStripEditor::timerCallback()
{
    float grDb = std::abs(processor.getGainReduction());
    grNeedleMeter.setGainReduction(grDb);
    grNeedleMeter.advancePhysics();

    inputMeter.updateLevel(processor.getInputPeak());
    outputMeter.updateLevel(processor.getOutputPeak());
    inputMeter.repaint();
    outputMeter.repaint();
}

//==============================================================================
void ModulatedStripEditor::paint(juce::Graphics& g)
{
    int W = getWidth();
    int H = getHeight();

    g.fillAll(juce::Colour(0xFF080808));

    // Wood panels
    PanelTextures::drawWoodPanel(g, {0, 0, 24, (float)H});
    PanelTextures::drawWoodPanel(g, {(float)W-24, 0, 24, (float)H});

    // Header
    g.setColour(juce::Colour(0xFF0F0F0F));
    g.fillRect(24, 0, W-48, 48);
    g.setColour(juce::Colour(0xFF7A4A10));
    g.fillRect(24, 46, W-48, 2);
    g.setColour(juce::Colour(0xFFE8A838));
    for (int x = 24; x < W-24; x += 8) g.fillRect(x, 46, 4, 2);
    g.setFont(juce::Font(juce::FontOptions(20.0f).withStyle("Bold")));
    PanelTextures::drawEngravedText(g, "MODULATED STRIP", {24, 4, W-48, 36});
    g.setColour(juce::Colour(0xFF444444));
    g.setFont(juce::Font(juce::FontOptions(7.0f)));
    g.drawText("ANALOG CHANNEL PROCESSOR", 24, 26, W-48, 14, juce::Justification::centred);

    // Footer
    g.setColour(juce::Colour(0xFF0F0F0F));
    g.fillRect(24, H-24, W-48, 24);
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.fillRect(24, H-24, W-48, 1);

    // Footer labels
    g.setColour(juce::Colour(0xFF555555));
    g.setFont(juce::Font(juce::FontOptions(7.0f)));
    g.drawText("STEREO", 34, H-22, 48, 12, juce::Justification::centredLeft);
    g.drawText("QUALITY", 200, H-22, 50, 12, juce::Justification::centredLeft);
    g.drawText("CHARACTER", 505, H-22, 80, 12,juce::Justification::centredLeft);
    float pT = 48.0f;
    float pH = (float)H - 72;

    // INPUT
    PanelTextures::drawBrushedMetal(g, {28, pT, 88, pH}, juce::Colour(0xFF141414), 0.02f);
    PanelTextures::drawScrewHead(g, 36, pT+8, 0.3f);
    PanelTextures::drawScrewHead(g, 108, pT+8, 1.2f);
    PanelTextures::drawScrewHead(g, 36, pT+pH-8, 0.8f);
    PanelTextures::drawScrewHead(g, 108, pT+pH-8, 2.0f);
    g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
    PanelTextures::drawEngravedText(g, "INPUT", {28, (int)pT+4, 88, 16});

    // SATURATION
    PanelTextures::drawBrushedMetal(g, {120, pT, 200, pH}, getSatPanelColor(currentSatModel), 0.025f);
    PanelTextures::drawScrewHead(g, 128, pT+8, 0.5f);
    PanelTextures::drawScrewHead(g, 312, pT+8, 1.8f);
    PanelTextures::drawScrewHead(g, 128, pT+pH-8, 1.1f);
    PanelTextures::drawScrewHead(g, 312, pT+pH-8, 0.2f);
    PanelTextures::drawEngravedText(g, "SATURATION", {120, (int)pT+4, 200, 16});

    // COMPRESSOR
    auto compColor = getCompPanelColor(currentCompModel);
    if (currentCompModel == 1)
        PanelTextures::drawWrinkleFinish(g, {324, pT, 380, pH}, compColor);
    else
        PanelTextures::drawBrushedMetal(g, {324, pT, 380, pH}, compColor, 0.02f);
    PanelTextures::drawScrewHead(g, 332, pT+8, 0.9f);
    PanelTextures::drawScrewHead(g, 696, pT+8, 1.5f);
    PanelTextures::drawScrewHead(g, 332, pT+pH-8, 2.2f);
    PanelTextures::drawScrewHead(g, 696, pT+pH-8, 0.4f);
    if (currentCompModel == 0) { g.setColour(juce::Colour(0xFF4060A0)); g.fillRect(324.0f, pT, 380.0f, 2.0f); }
    PanelTextures::drawEngravedText(g, "COMPRESSOR", {324, (int)pT+4, 380, 16});

    // EQ
    PanelTextures::drawBrushedMetal(g, {708, pT, 440, pH}, getEQPanelColor(currentEQModel), 0.025f);
    PanelTextures::drawScrewHead(g, 716, pT+8, 1.3f);
    PanelTextures::drawScrewHead(g, 1140, pT+8, 0.6f);
    PanelTextures::drawScrewHead(g, 716, pT+pH-8, 1.9f);
    PanelTextures::drawScrewHead(g, 1140, pT+pH-8, 0.1f);
    if (currentEQModel == 0 || currentEQModel == 1)
    { g.setColour(juce::Colour(0xFFE8A838)); g.fillRect(708.0f, pT, 440.0f, 2.0f); }
    else if (currentEQModel == 2)
    {
        g.setColour(juce::Colour(0xFF4060A0)); g.fillRect(708.0f, pT, 440.0f, 1.0f);
        g.setColour(juce::Colour(0xFFC03030)); g.fillRect(708.0f, pT+1, 440.0f, 1.0f);
    }
    PanelTextures::drawEngravedText(g, "EQUALIZER", {708, (int)pT+4, 440, 16});

    // OUTPUT
    PanelTextures::drawBrushedMetal(g, {1152, pT, 88, pH}, juce::Colour(0xFF141414), 0.02f);
    PanelTextures::drawScrewHead(g, 1160, pT+8, 1.7f);
    PanelTextures::drawScrewHead(g, 1232, pT+8, 0.3f);
    PanelTextures::drawScrewHead(g, 1160, pT+pH-8, 0.5f);
    PanelTextures::drawScrewHead(g, 1232, pT+pH-8, 2.3f);
    PanelTextures::drawEngravedText(g, "OUTPUT", {1152, (int)pT+4, 88, 16});

    // Dust
    PanelTextures::drawDust(g, getLocalBounds().toFloat(), 0.006f);
}

//==============================================================================
void ModulatedStripEditor::resized()
{
    int k = 68, lh = 14;

    // Header
    presetBar.setBounds(300, 6, 680, 36);

    if (presetBrowser != nullptr && presetBrowser->isVisible())
        presetBrowser->setBounds(getWidth()/2-210, getHeight()/2-260, 420, 520);

    // INPUT
    inputGainKnob.setBounds(38, 72, k+lh, k+lh);
    inputMeterLabel.setBounds(38, 72+k+lh+6, k+lh, 12);
    inputMeter.setBounds(52, 72+k+lh+20, 18, 380);

    // SATURATION
    satModelSelector.setBounds(130, 68, 175, 24);
    satBypassBtn.setBounds(268, 96, 42, 20);
    driveKnob.setBounds(130, 128, k+lh, k+lh);
    satMixKnob.setBounds(220, 128, k+lh, k+lh);

    // COMPRESSOR
    int cx = 334;
    compModelSelector.setBounds(cx, 68, 210, 24);
    compBypassBtn.setBounds(cx+220, 71, 42, 20);
    compModelHintLabel.setBounds(cx, 95, 260, 12);
    grNeedleMeter.setBounds(cx+20, 110, 280, 110);
    fairchildTCSelector.setBounds(cx, 226, 260, 24);
    allInBtn.setBounds(cx+210, 226, 65, 22);
    la2aLimitBtn.setBounds(cx+210, 226, 55, 22);
    thrustBtn.setBounds(cx+160, 226, 65, 22);
    topologyBtn.setBounds(cx+230, 226, 55, 22);

    int cy = 256;
    thresholdKnob.setBounds(cx, cy, k+lh, k+lh);
    ratioKnob.setBounds(cx+110, cy, k+lh, k+lh);
    kneeKnob.setBounds(cx+220, cy, k+lh, k+lh);
    cy += k+lh+12;
    attackKnob.setBounds(cx, cy, k+lh, k+lh);
    releaseKnob.setBounds(cx+110, cy, k+lh, k+lh);
    cy += k+lh+12;
    makeupKnob.setBounds(cx, cy, k+lh, k+lh);
    compMixKnob.setBounds(cx+110, cy, k+lh, k+lh);

    // EQ
    int ex = 718;
    eqModelSelector.setBounds(ex, 68, 210, 24);
    eqBypassBtn.setBounds(ex+220, 71, 42, 20);
    eqPreCompBtn.setBounds(ex+268, 71, 60, 20);
    eqModelHintLabel.setBounds(ex, 95, 320, 12);

    int ey = 115;
    eqLowGainKnob.setBounds(ex, ey, k+lh, k+lh);
    eqLowFreqKnob.setBounds(ex+105, ey, k+lh, k+lh);
    ey += k+lh+12;
    eqMidGainKnob.setBounds(ex, ey, k+lh, k+lh);
    eqMidFreqKnob.setBounds(ex+105, ey, k+lh, k+lh);
    eqMidQKnob.setBounds(ex+210, ey, k+lh, k+lh);
    ey += k+lh+12;
    eqHighGainKnob.setBounds(ex, ey, k+lh, k+lh);
    eqHighFreqKnob.setBounds(ex+105, ey, k+lh, k+lh);
    eqHPFKnob.setBounds(ex+315, ey, k+lh, k+lh);

    // OUTPUT
    outputGainKnob.setBounds(1160, 72, k+lh, k+lh);
    outputMeterLabel.setBounds(1160, 72+k+lh+6, k+lh, 12);
    outputMeter.setBounds(1180, 72+k+lh+20, 18, 380);

    // FOOTER CONTROLS
    int fy = getHeight() - 22;
    stereoModeSelector.setBounds(84, fy, 105, 20);
    oversampleSelector.setBounds(260, fy, 90, 20);
    deltaBtn.setBounds(365, fy, 55, 20);
    analogBypassBtn.setBounds(430, fy, 65, 20);
    crosstalkBtn .setBounds(510, fy, 55, 20);
    noiseFloorBtn.setBounds(577, fy, 55, 20);
}