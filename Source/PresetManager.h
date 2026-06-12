#pragma once
#include <JuceHeader.h>

//==============================================================================
// PRESET DATA
// One preset = all parameter values
//==============================================================================
struct ModulatedStripPreset
{
    juce::String name;
    juce::String category;
    juce::String description;
    juce::String author;

    // Stored as XML state from APVTS
    std::unique_ptr<juce::XmlElement> state;

    // Display info
    juce::String satModel;
    juce::String compModel;
    juce::String eqModel;
};

//==============================================================================
// PRESET MANAGER
// Handles factory and user presets
//==============================================================================
class PresetManager
{
public:
    PresetManager(juce::AudioProcessorValueTreeState& apvts)
        : apvts(apvts)
    {
        buildFactoryPresets();
    }

    //──────────────────────────────────────────
    // CATEGORIES
    //──────────────────────────────────────────
    juce::StringArray getCategories() const
    {
        juce::StringArray cats;
        cats.add("All");
        for (auto& preset : factoryPresets)
        {
            if (!cats.contains(preset.category))
                cats.add(preset.category);
        }
        for (auto& preset : userPresets)
        {
            if (!cats.contains(preset.category))
                cats.add(preset.category);
        }
        return cats;
    }

    //──────────────────────────────────────────
    // GET PRESETS BY CATEGORY
    //──────────────────────────────────────────
    juce::Array<const ModulatedStripPreset*>
    getPresetsForCategory(
        const juce::String& category) const
    {
        juce::Array<const ModulatedStripPreset*> result;

        for (auto& p : factoryPresets)
        {
            if (category == "All"
             || p.category == category)
                result.add(&p);
        }
        for (auto& p : userPresets)
        {
            if (category == "All"
             || p.category == category)
                result.add(&p);
        }
        return result;
    }

    //──────────────────────────────────────────
    // LOAD PRESET
    //──────────────────────────────────────────
    void loadPreset(const ModulatedStripPreset* preset)
    {
        if (!preset || !preset->state) return;

        apvts.replaceState(
            juce::ValueTree::fromXml(*preset->state));
        currentPresetName = preset->name;
        currentPresetCategory = preset->category;
    }

    //──────────────────────────────────────────
    // SAVE USER PRESET
    //──────────────────────────────────────────
    void saveUserPreset(
        const juce::String& name,
        const juce::String& category,
        const juce::String& description)
    {
        ModulatedStripPreset preset;
        preset.name        = name;
        preset.category    = category;
        preset.description = description;
        preset.author      = "User";

        auto state = apvts.copyState();
        preset.state = state.createXml();

        // Update if exists
        for (auto& p : userPresets)
        {
            if (p.name == name)
            {
                p = std::move(preset);
                saveUserPresetsToFile();
                currentPresetName = name;
                return;
            }
        }

        userPresets.push_back(std::move(preset));
        saveUserPresetsToFile();
        currentPresetName = name;
    }

    //──────────────────────────────────────────
    // DELETE USER PRESET
    //──────────────────────────────────────────
    void deleteUserPreset(const juce::String& name)
    {
        userPresets.erase(
            std::remove_if(userPresets.begin(),
                userPresets.end(),
                [&](const ModulatedStripPreset& p) {
                    return p.name == name;
                }),
            userPresets.end());
        saveUserPresetsToFile();
    }

    //──────────────────────────────────────────
    // NAVIGATION
    //──────────────────────────────────────────
    void loadNextPreset()
    {
        auto all = getPresetsForCategory("All");
        if (all.isEmpty()) return;

        int idx = getCurrentPresetIndex(all);
        int next = (idx + 1) % all.size();
        loadPreset(all[next]);
    }

    void loadPreviousPreset()
    {
        auto all = getPresetsForCategory("All");
        if (all.isEmpty()) return;

        int idx  = getCurrentPresetIndex(all);
        int prev = (idx - 1 + all.size()) % all.size();
        loadPreset(all[prev]);
    }

    juce::String getCurrentPresetName() const
    {
        return currentPresetName;
    }

    juce::String getCurrentPresetCategory() const
    {
        return currentPresetCategory;
    }

    void loadUserPresetsFromFile()
    {
        auto file = getUserPresetsFile();
        if (!file.existsAsFile()) return;

        auto xml = juce::XmlDocument::parse(file);
        if (!xml) return;

        userPresets.clear();
        for (auto* presetEl :
             xml->getChildIterator())
        {
            ModulatedStripPreset p;
            p.name        = presetEl->getStringAttribute(
                "name");
            p.category    = presetEl->getStringAttribute(
                "category", "User");
            p.description = presetEl->getStringAttribute(
                "description");
            p.author      = presetEl->getStringAttribute(
                "author", "User");

            if (auto* stateEl =
                presetEl->getChildElement(0))
            {
                p.state = std::make_unique
                    <juce::XmlElement>(*stateEl);
            }

            if (p.state)
                userPresets.push_back(std::move(p));
        }
    }

private:
    juce::AudioProcessorValueTreeState& apvts;

    std::vector<ModulatedStripPreset> factoryPresets;
    std::vector<ModulatedStripPreset> userPresets;

    juce::String currentPresetName     = "Default";
    juce::String currentPresetCategory = "Init";

    //──────────────────────────────────────────
    // FILE MANAGEMENT
    //──────────────────────────────────────────
    juce::File getUserPresetsFile() const
    {
        return juce::File::getSpecialLocation(
            juce::File::userApplicationDataDirectory)
            .getChildFile("ModulatedStrip")
            .getChildFile("UserPresets.xml");
    }

    void saveUserPresetsToFile()
    {
        auto file = getUserPresetsFile();
        file.getParentDirectory().createDirectory();

        juce::XmlElement root("UserPresets");
        for (auto& p : userPresets)
        {
            auto* presetEl = root.createNewChildElement(
                "Preset");
            presetEl->setAttribute("name", p.name);
            presetEl->setAttribute("category",
                p.category);
            presetEl->setAttribute("description",
                p.description);
            presetEl->setAttribute("author",
                p.author);

            if (p.state)
                presetEl->addChildElement(
                    new juce::XmlElement(*p.state));
        }

        root.writeTo(file);
    }

    int getCurrentPresetIndex(
        const juce::Array<
            const ModulatedStripPreset*>& list) const
    {
        for (int i = 0; i < list.size(); i++)
        {
            if (list[i]->name == currentPresetName)
                return i;
        }
        return 0;
    }

    //──────────────────────────────────────────
    // FACTORY PRESET BUILDER
    //──────────────────────────────────────────
    void buildFactoryPresets()
    {
        // Each preset defined as parameter value map
        // then converted to APVTS state

        // ── INIT ──────────────────────────────
        addFactory("Init",
            "Init",
            "Clean passthrough - all processing neutral",
            {
                {"drive",          0.0f},
                {"satMix",         100.0f},
                {"satModel",       0.0f},
                {"compThreshold",  -20.0f},
                {"compRatio",      1.0f},
                {"compAttack",     10.0f},
                {"compRelease",    100.0f},
                {"compMakeup",     0.0f},
                {"compMix",        0.0f},
                {"eqLowGain",      0.0f},
                {"eqMidGain",      0.0f},
                {"eqHighGain",     0.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        // ── DEEP HOUSE ────────────────────────
        addFactory("Deep House Bus",
            "Deep House",
            "Classic deep house mix bus glue. "
            "Neve warmth into SSL compression. "
            "Subtle air and low end bloom.",
            {
                {"satModel",       0.0f},  // NEVE
                {"drive",          18.0f},
                {"satMix",         85.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -18.0f},
                {"compRatio",      2.0f},
                {"compAttack",     3.0f},
                {"compRelease",    400.0f},
                {"compMakeup",     2.0f},
                {"compMix",        70.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      2.5f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -1.5f},
                {"eqMidFreq",      350.0f},
                {"eqHighGain",     2.0f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -1.0f}
            });

        addFactory("Deep House Kick",
            "Deep House",
            "Punch and thump for deep house kicks. "
            "FET saturation, 1176 snap, "
            "API low boost.",
            {
                {"satModel",       5.0f},  // FET
                {"drive",          35.0f},
                {"satMix",         75.0f},
                {"compModel",      3.0f},  // 1176
                {"compThreshold",  -12.0f},
                {"compRatio",      8.0f},
                {"compAttack",     2.0f},
                {"compRelease",    80.0f},
                {"compMakeup",     4.0f},
                {"compMix",        85.0f},
                {"eqModel",        4.0f},  // API 550A
                {"eqLowGain",      4.0f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -2.5f},
                {"eqMidFreq",      400.0f},
                {"eqHighGain",     1.5f},
                {"eqHighFreq",     8000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        addFactory("Deep House Bass",
            "Deep House",
            "Sub bass warmth and control. "
            "Tube saturation for harmonic richness. "
            "LA-2A optical smoothness. "
            "Pultec 60Hz bloom.",
            {
                {"satModel",       3.0f},  // TUBE
                {"drive",          22.0f},
                {"satMix",         90.0f},
                {"compModel",      2.0f},  // LA-2A
                {"compThreshold",  -15.0f},
                {"compRatio",      3.0f},
                {"compAttack",     10.0f},
                {"compRelease",    300.0f},
                {"compMakeup",     3.0f},
                {"compMix",        100.0f},
                {"eqModel",        3.0f},  // Pultec
                {"eqLowGain",      5.0f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -1.0f},
                {"eqMidFreq",      300.0f},
                {"eqHighGain",     1.0f},
                {"eqHighFreq",     8000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -1.0f}
            });

        addFactory("Deep Synth Pads",
            "Deep House",
            "Wide warm pads with transformer color. "
            "Fairchild glue for sustain. "
            "Neve air and low warmth.",
            {
                {"satModel",       6.0f},  // IRON
                {"drive",          12.0f},
                {"satMix",         80.0f},
                {"compModel",      1.0f},  // Fairchild
                {"compThreshold",  -20.0f},
                {"compRatio",      3.0f},
                {"compAttack",     20.0f},
                {"compRelease",    800.0f},
                {"compMakeup",     2.0f},
                {"compMix",        60.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      2.0f},
                {"eqLowFreq",      110.0f},
                {"eqMidGain",      -1.0f},
                {"eqMidFreq",      500.0f},
                {"eqHighGain",     3.0f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        // ── PROGRESSIVE HOUSE ─────────────────
        addFactory("Progressive Bus",
            "Progressive House",
            "Modern progressive house mix bus. "
            "SSL precision with API punch. "
            "Clean and wide.",
            {
                {"satModel",       1.0f},  // SSL
                {"drive",          10.0f},
                {"satMix",         70.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -15.0f},
                {"compRatio",      4.0f},
                {"compAttack",     1.5f},
                {"compRelease",    200.0f},
                {"compMakeup",     2.5f},
                {"compMix",        80.0f},
                {"eqModel",        2.0f},  // SSL 4000E
                {"eqLowGain",      1.5f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -1.0f},
                {"eqMidFreq",      800.0f},
                {"eqHighGain",     2.5f},
                {"eqHighFreq",     16000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -0.5f}
            });

        addFactory("Progressive Lead",
            "Progressive House",
            "Forward melodic lead synth. "
            "API aggression with presence. "
            "Cuts through the mix.",
            {
                {"satModel",       2.0f},  // API
                {"drive",          30.0f},
                {"satMix",         80.0f},
                {"compModel",      3.0f},  // 1176
                {"compThreshold",  -10.0f},
                {"compRatio",      4.0f},
                {"compAttack",     5.0f},
                {"compRelease",    120.0f},
                {"compMakeup",     5.0f},
                {"compMix",        90.0f},
                {"eqModel",        4.0f},  // API 550A
                {"eqLowGain",      -1.5f},
                {"eqLowFreq",      100.0f},
                {"eqMidGain",      3.0f},
                {"eqMidFreq",      2000.0f},
                {"eqHighGain",     2.0f},
                {"eqHighFreq",     10000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        addFactory("Progressive Pluck",
            "Progressive House",
            "Punchy arp and pluck sounds. "
            "FET snap with SSL definition.",
            {
                {"satModel",       5.0f},  // FET
                {"drive",          25.0f},
                {"satMix",         70.0f},
                {"compModel",      4.0f},  // API 2500
                {"compThreshold",  -14.0f},
                {"compRatio",      4.0f},
                {"compAttack",     0.5f},
                {"compRelease",    80.0f},
                {"compMakeup",     4.0f},
                {"compMix",        85.0f},
                {"eqModel",        2.0f},  // SSL 4000E
                {"eqLowGain",      -2.0f},
                {"eqLowFreq",      100.0f},
                {"eqMidGain",      2.0f},
                {"eqMidFreq",      3000.0f},
                {"eqHighGain",     3.0f},
                {"eqHighFreq",     10000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        // ── MELODIC HOUSE ─────────────────────
        addFactory("Melodic Bus",
            "Melodic House",
            "Warm melodic house mix bus. "
            "Tape saturation for cohesion. "
            "Fairchild for musical dynamics. "
            "Pultec vintage air.",
            {
                {"satModel",       4.0f},  // TAPE
                {"drive",          15.0f},
                {"satMix",         75.0f},
                {"compModel",      1.0f},  // Fairchild
                {"compThreshold",  -22.0f},
                {"compRatio",      3.0f},
                {"compAttack",     30.0f},
                {"compRelease",    800.0f},
                {"compMakeup",     1.5f},
                {"compMix",        65.0f},
                {"eqModel",        3.0f},  // Pultec
                {"eqLowGain",      2.0f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -0.5f},
                {"eqMidFreq",      500.0f},
                {"eqHighGain",     3.0f},
                {"eqHighFreq",     16000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -0.5f}
            });

        addFactory("Melodic Chord Stab",
            "Melodic House",
            "Warm chord stabs with vintage color. "
            "Neve transformer richness. "
            "Smooth LA-2A leveling.",
            {
                {"satModel",       0.0f},  // NEVE
                {"drive",          20.0f},
                {"satMix",         85.0f},
                {"compModel",      2.0f},  // LA-2A
                {"compThreshold",  -18.0f},
                {"compRatio",      3.0f},
                {"compAttack",     10.0f},
                {"compRelease",    500.0f},
                {"compMakeup",     3.0f},
                {"compMix",        90.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      2.5f},
                {"eqLowFreq",      110.0f},
                {"eqMidGain",      -1.0f},
                {"eqMidFreq",      600.0f},
                {"eqHighGain",     2.5f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        addFactory("Melodic Arp",
            "Melodic House",
            "Bright melodic arpeggios. "
            "SSL precision and clarity. "
            "API forward mids.",
            {
                {"satModel",       1.0f},  // SSL
                {"drive",          15.0f},
                {"satMix",         75.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -16.0f},
                {"compRatio",      3.0f},
                {"compAttack",     2.0f},
                {"compRelease",    150.0f},
                {"compMakeup",     3.5f},
                {"compMix",        80.0f},
                {"eqModel",        4.0f},  // API 550A
                {"eqLowGain",      -1.5f},
                {"eqLowFreq",      100.0f},
                {"eqMidGain",      2.5f},
                {"eqMidFreq",      2500.0f},
                {"eqHighGain",     3.5f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        // ── DRUM BUS ──────────────────────────
        addFactory("Drum Bus Punch",
            "Drums",
            "Aggressive drum bus treatment. "
            "API saturation for bite. "
            "Fast SSL glue compression. "
            "Presence and snap.",
            {
                {"satModel",       2.0f},  // API
                {"drive",          28.0f},
                {"satMix",         80.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -14.0f},
                {"compRatio",      4.0f},
                {"compAttack",     0.3f},
                {"compRelease",    100.0f},
                {"compMakeup",     4.0f},
                {"compMix",        85.0f},
                {"eqModel",        4.0f},  // API 550A
                {"eqLowGain",      3.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -2.0f},
                {"eqMidFreq",      400.0f},
                {"eqHighGain",     2.5f},
                {"eqHighFreq",     8000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -1.0f}
            });

        addFactory("Drum Bus Vintage",
            "Drums",
            "Classic vintage drum sound. "
            "Tape saturation cohesion. "
            "Fairchild smoothing. "
            "Neve character.",
            {
                {"satModel",       4.0f},  // TAPE
                {"drive",          20.0f},
                {"satMix",         85.0f},
                {"compModel",      1.0f},  // Fairchild
                {"compThreshold",  -20.0f},
                {"compRatio",      3.0f},
                {"compAttack",     10.0f},
                {"compRelease",    500.0f},
                {"compMakeup",     2.5f},
                {"compMix",        70.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      3.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -2.5f},
                {"eqMidFreq",      350.0f},
                {"eqHighGain",     2.0f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -1.0f}
            });

        // ── VOCALS ────────────────────────────
        addFactory("Vocal Smooth",
            "Vocals",
            "Smooth transparent vocal chain. "
            "Subtle tube warmth. "
            "LA-2A natural leveling. "
            "Neve presence.",
            {
                {"satModel",       3.0f},  // TUBE
                {"drive",          12.0f},
                {"satMix",         70.0f},
                {"compModel",      2.0f},  // LA-2A
                {"compThreshold",  -20.0f},
                {"compRatio",      3.0f},
                {"compAttack",     10.0f},
                {"compRelease",    400.0f},
                {"compMakeup",     4.0f},
                {"compMix",        100.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      0.0f},
                {"eqLowFreq",      100.0f},
                {"eqMidGain",      1.5f},
                {"eqMidFreq",      3000.0f},
                {"eqHighGain",     2.5f},
                {"eqHighFreq",     12000.0f},
                {"eqHPF",          80.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        addFactory("Vocal Aggressive",
            "Vocals",
            "Punchy forward vocal. "
            "FET edge and presence. "
            "1176 fast control. "
            "API forward midrange.",
            {
                {"satModel",       5.0f},  // FET
                {"drive",          25.0f},
                {"satMix",         75.0f},
                {"compModel",      3.0f},  // 1176
                {"compThreshold",  -18.0f},
                {"compRatio",      4.0f},
                {"compAttack",     1.0f},
                {"compRelease",    80.0f},
                {"compMakeup",     5.0f},
                {"compMix",        90.0f},
                {"eqModel",        4.0f},  // API 550A
                {"eqLowGain",      -2.0f},
                {"eqLowFreq",      100.0f},
                {"eqMidGain",      3.0f},
                {"eqMidFreq",      3500.0f},
                {"eqHighGain",     3.0f},
                {"eqHighFreq",     10000.0f},
                {"eqHPF",          100.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        // ── LIVE PERFORMANCE ─────────────────
        addFactory("Live Set Opener",
            "Live Performance",
            "Gentle analog warmth for set opening. "
            "Iron transformer color only. "
            "Light SSL glue. "
            "Natural Neve tone.",
            {
                {"satModel",       6.0f},  // IRON
                {"drive",          8.0f},
                {"satMix",         60.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -25.0f},
                {"compRatio",      2.0f},
                {"compAttack",     10.0f},
                {"compRelease",    400.0f},
                {"compMakeup",     1.0f},
                {"compMix",        50.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      1.5f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      0.0f},
                {"eqMidFreq",      1000.0f},
                {"eqHighGain",     1.5f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        addFactory("Live Peak Time",
            "Live Performance",
            "Maximum energy for peak time. "
            "Aggressive FET saturation. "
            "1176 all-in compression. "
            "API punchy EQ.",
            {
                {"satModel",       5.0f},  // FET
                {"drive",          45.0f},
                {"satMix",         85.0f},
                {"compModel",      3.0f},  // 1176
                {"compThreshold",  -12.0f},
                {"compRatio",      8.0f},
                {"compAttack",     0.5f},
                {"compRelease",    60.0f},
                {"compMakeup",     6.0f},
                {"compMix",        90.0f},
                {"eqModel",        4.0f},  // API 550A
                {"eqLowGain",      4.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -2.0f},
                {"eqMidFreq",      500.0f},
                {"eqHighGain",     3.0f},
                {"eqHighFreq",     10000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -2.0f}
            });

        addFactory("Live Cool Down",
            "Live Performance",
            "Smooth landing after peak time. "
            "Tube warmth and vintage tone. "
            "Fairchild musical dynamics. "
            "Pultec vintage air.",
            {
                {"satModel",       3.0f},  // TUBE
                {"drive",          15.0f},
                {"satMix",         80.0f},
                {"compModel",      1.0f},  // Fairchild
                {"compThreshold",  -22.0f},
                {"compRatio",      3.0f},
                {"compAttack",     40.0f},
                {"compRelease",    1200.0f},
                {"compMakeup",     2.0f},
                {"compMix",        65.0f},
                {"eqModel",        3.0f},  // Pultec
                {"eqLowGain",      2.5f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -0.5f},
                {"eqMidFreq",      400.0f},
                {"eqHighGain",     3.5f},
                {"eqHighFreq",     16000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        addFactory("Live Mix Bus",
            "Live Performance",
            "Professional live mix bus. "
            "Tape cohesion. "
            "SSL controlled dynamics. "
            "Neve natural tone.",
            {
                {"satModel",       4.0f},  // TAPE
                {"drive",          12.0f},
                {"satMix",         70.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -18.0f},
                {"compRatio",      2.0f},
                {"compAttack",     3.0f},
                {"compRelease",    300.0f},
                {"compMakeup",     1.5f},
                {"compMix",        60.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      1.5f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -1.0f},
                {"eqMidFreq",      400.0f},
                {"eqHighGain",     1.5f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -1.0f}
            });

        // ── MASTERING ────────────────────────
        addFactory("Analog Master",
            "Mastering",
            "Full mastering chain with vintage character. "
            "Neve transformer into Fairchild. "
            "Pultec classic EQ.",
            {
                {"satModel",       0.0f},  // NEVE
                {"drive",          8.0f},
                {"satMix",         65.0f},
                {"compModel",      1.0f},  // Fairchild
                {"compThreshold",  -24.0f},
                {"compRatio",      3.0f},
                {"compAttack",     30.0f},
                {"compRelease",    1000.0f},
                {"compMakeup",     1.0f},
                {"compMix",        55.0f},
                {"eqModel",        3.0f},  // Pultec
                {"eqLowGain",      1.5f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -0.5f},
                {"eqMidFreq",      300.0f},
                {"eqHighGain",     2.0f},
                {"eqHighFreq",     16000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -0.5f}
            });

        addFactory("Modern Master",
            "Mastering",
            "Clean modern mastering chain. "
            "SSL precision throughout. "
            "Controlled and transparent.",
            {
                {"satModel",       1.0f},  // SSL
                {"drive",          6.0f},
                {"satMix",         60.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -20.0f},
                {"compRatio",      2.0f},
                {"compAttack",     5.0f},
                {"compRelease",    500.0f},
                {"compMakeup",     1.0f},
                {"compMix",        50.0f},
                {"eqModel",        2.0f},  // SSL 4000E
                {"eqLowGain",      1.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -0.5f},
                {"eqMidFreq",      500.0f},
                {"eqHighGain",     1.5f},
                {"eqHighFreq",     16000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -0.5f}
            });

        // ── VOCALS EXTENDED ──────────────────
        addFactory("Radio Vocal",
            "Vocals",
            "Bright, controlled, sits in the mix. "
            "SSL precision with API presence.",
            {
                {"satModel",       1.0f},  // SSL
                {"drive",          15.0f},
                {"satMix",         70.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -18.0f},
                {"compRatio",      4.0f},
                {"compAttack",     2.0f},
                {"compRelease",    150.0f},
                {"compMakeup",     4.0f},
                {"compMix",        90.0f},
                {"eqModel",        4.0f},  // API 550A
                {"eqLowGain",      -3.0f},
                {"eqLowFreq",      100.0f},
                {"eqMidGain",      2.5f},
                {"eqMidFreq",      3000.0f},
                {"eqHighGain",     4.0f},
                {"eqHighFreq",     10000.0f},
                {"eqHPF",          100.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        addFactory("Vocal Warmth",
            "Vocals",
            "Vintage warmth for intimate vocals. "
            "Tube saturation, Fairchild smoothing, "
            "Neve presence.",
            {
                {"satModel",       3.0f},  // TUBE
                {"drive",          18.0f},
                {"satMix",         80.0f},
                {"compModel",      1.0f},  // Fairchild
                {"compThreshold",  -22.0f},
                {"compRatio",      3.0f},
                {"compAttack",     25.0f},
                {"compRelease",    600.0f},
                {"compMakeup",     3.0f},
                {"compMix",        80.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      0.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      2.0f},
                {"eqMidFreq",      2500.0f},
                {"eqHighGain",     2.5f},
                {"eqHighFreq",     12000.0f},
                {"eqHPF",          80.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        // ── SYNTH SPECIFIC ───────────────────
        addFactory("Synth Bass Punch",
            "Synths",
            "Maximum punch for synth bass. "
            "FET saturation, fast 1176 control, "
            "API low boost.",
            {
                {"satModel",       5.0f},  // FET
                {"drive",          40.0f},
                {"satMix",         65.0f},
                {"compModel",      3.0f},  // 1176
                {"compThreshold",  -10.0f},
                {"compRatio",      8.0f},
                {"compAttack",     1.0f},
                {"compRelease",    60.0f},
                {"compMakeup",     5.0f},
                {"compMix",        60.0f},
                {"eqModel",        4.0f},  // API 550A
                {"eqLowGain",      4.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -2.0f},
                {"eqMidFreq",      500.0f},
                {"eqHighGain",     2.0f},
                {"eqHighFreq",     8000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -1.0f}
            });

        addFactory("Analog Synth Color",
            "Synths",
            "Add vintage transformer color to "
            "digital synths. Neve warmth throughout.",
            {
                {"satModel",       0.0f},  // NEVE
                {"drive",          25.0f},
                {"satMix",         90.0f},
                {"compModel",      1.0f},  // Fairchild
                {"compThreshold",  -24.0f},
                {"compRatio",      3.0f},
                {"compAttack",     15.0f},
                {"compRelease",    500.0f},
                {"compMakeup",     2.0f},
                {"compMix",        70.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      2.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -1.0f},
                {"eqMidFreq",      400.0f},
                {"eqHighGain",     2.0f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        addFactory("808 Treatment",
            "Synths",
            "808 sub bass control and shape. "
            "Tube harmonics, smooth LA-2A, "
            "Pultec boom.",
            {
                {"satModel",       3.0f},  // TUBE
                {"drive",          20.0f},
                {"satMix",         85.0f},
                {"compModel",      2.0f},  // LA-2A
                {"compThreshold",  -12.0f},
                {"compRatio",      3.0f},
                {"compAttack",     15.0f},
                {"compRelease",    400.0f},
                {"compMakeup",     2.0f},
                {"compMix",        100.0f},
                {"eqModel",        3.0f},  // Pultec
                {"eqLowGain",      4.0f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -1.5f},
                {"eqMidFreq",      300.0f},
                {"eqHighGain",     0.0f},
                {"eqHighFreq",     8000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -1.0f}
            });

        // ── MASTERING EXTENDED ───────────────
        addFactory("Vintage Master",
            "Mastering",
            "Full vintage mastering chain. "
            "Neve iron into Fairchild into Pultec. "
            "The classic 1970s master.",
            {
                {"satModel",       6.0f},  // IRON
                {"drive",          10.0f},
                {"satMix",         70.0f},
                {"compModel",      1.0f},  // Fairchild
                {"compThreshold",  -22.0f},
                {"compRatio",      3.0f},
                {"compAttack",     30.0f},
                {"compRelease",    1200.0f},
                {"compMakeup",     1.5f},
                {"compMix",        60.0f},
                {"eqModel",        3.0f},  // Pultec
                {"eqLowGain",      1.5f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -0.5f},
                {"eqMidFreq",      400.0f},
                {"eqHighGain",     2.5f},
                {"eqHighFreq",     16000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -0.5f}
            });

        addFactory("Streaming Master",
            "Mastering",
            "Optimized for streaming loudness targets. "
            "SSL control, Neve air. "
            "Targets approximately -14 LUFS.",
            {
                {"satModel",       4.0f},  // TAPE
                {"drive",          8.0f},
                {"satMix",         65.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -18.0f},
                {"compRatio",      2.0f},
                {"compAttack",     5.0f},
                {"compRelease",    500.0f},
                {"compMakeup",     1.0f},
                {"compMix",        55.0f},
                {"eqModel",        2.0f},  // SSL 4000E
                {"eqLowGain",      1.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -0.5f},
                {"eqMidFreq",      500.0f},
                {"eqHighGain",     1.5f},
                {"eqHighFreq",     16000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -1.0f}
            });

        // ── CREATIVE / EFFECT ────────────────
        addFactory("Heavy Saturation",
            "Creative",
            "Heavy analog saturation for effect. "
            "FET into 1176 all-in. "
            "Aggressive parallel processing.",
            {
                {"satModel",       5.0f},  // FET
                {"drive",          65.0f},
                {"satMix",         50.0f},
                {"compModel",      3.0f},  // 1176
                {"compThreshold",  -8.0f},
                {"compRatio",      12.0f},
                {"compAttack",     0.5f},
                {"compRelease",    50.0f},
                {"compMakeup",     6.0f},
                {"compMix",        40.0f},
                {"eqModel",        4.0f},  // API 550A
                {"eqLowGain",      3.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      2.0f},
                {"eqMidFreq",      2000.0f},
                {"eqHighGain",     2.0f},
                {"eqHighFreq",     8000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -3.0f}
            });

        addFactory("Tape Machine",
            "Creative",
            "Full tape machine simulation. "
            "Maximum tape saturation and "
            "Fairchild program dynamics.",
            {
                {"satModel",       4.0f},  // TAPE
                {"drive",          35.0f},
                {"satMix",         90.0f},
                {"compModel",      1.0f},  // Fairchild
                {"compThreshold",  -18.0f},
                {"compRatio",      3.0f},
                {"compAttack",     20.0f},
                {"compRelease",    800.0f},
                {"compMakeup",     2.0f},
                {"compMix",        80.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      2.0f},
                {"eqLowFreq",      110.0f},
                {"eqMidGain",      -1.5f},
                {"eqMidFreq",      350.0f},
                {"eqHighGain",     1.5f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -1.0f}
            });

        addFactory("Lo-Fi Treatment",
            "Creative",
            "Lo-fi vintage degradation. "
            "Heavy tube saturation, slow opto, "
            "vintage Pultec character.",
            {
                {"satModel",       3.0f},  // TUBE
                {"drive",          55.0f},
                {"satMix",         75.0f},
                {"compModel",      2.0f},  // LA-2A
                {"compThreshold",  -10.0f},
                {"compRatio",      10.0f},
                {"compAttack",     20.0f},
                {"compRelease",    1000.0f},
                {"compMakeup",     4.0f},
                {"compMix",        85.0f},
                {"eqModel",        3.0f},  // Pultec
                {"eqLowGain",      3.0f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -2.0f},
                {"eqMidFreq",      1000.0f},
                {"eqHighGain",     -3.0f},
                {"eqHighFreq",     8000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -2.0f}
            });

        // ── LIVE PERFORMANCE EXTENDED ─────────
        addFactory("Live Transition",
            "Live Performance",
            "Smooth transition between set sections. "
            "Iron transformer only, SSL light glue. "
            "Neutral and natural.",
            {
                {"satModel",       6.0f},  // IRON
                {"drive",          6.0f},
                {"satMix",         70.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -28.0f},
                {"compRatio",      1.5f},
                {"compAttack",     10.0f},
                {"compRelease",    500.0f},
                {"compMakeup",     0.5f},
                {"compMix",        45.0f},
                {"eqModel",        2.0f},  // SSL 4000E
                {"eqLowGain",      1.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      0.0f},
                {"eqMidFreq",      1000.0f},
                {"eqHighGain",     1.0f},
                {"eqHighFreq",     14000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        addFactory("Live Club Sound",
            "Live Performance",
            "Club PA optimization. "
            "API punch with THRUST bass protection. "
            "SSL air. Everything tight and loud.",
            {
                {"satModel",       2.0f},  // API
                {"drive",          20.0f},
                {"satMix",         75.0f},
                {"compModel",      4.0f},  // API 2500
                {"compThreshold",  -16.0f},
                {"compRatio",      4.0f},
                {"compAttack",     0.5f},
                {"compRelease",    120.0f},
                {"compMakeup",     3.0f},
                {"compMix",        80.0f},
                {"eqModel",        2.0f},  // SSL 4000E
                {"eqLowGain",      2.5f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -1.5f},
                {"eqMidFreq",      500.0f},
                {"eqHighGain",     2.5f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -1.5f}
            });

        // ── DEEP HOUSE EXTENDED ───────────────
        addFactory("Deep Modular",
            "Deep House",
            "Modular synth warmth. "
            "Heavy iron transformer into "
            "smooth Fairchild. Classic modular tone.",
            {
                {"satModel",       6.0f},  // IRON
                {"drive",          30.0f},
                {"satMix",         85.0f},
                {"compModel",      1.0f},  // Fairchild
                {"compThreshold",  -20.0f},
                {"compRatio",      3.0f},
                {"compAttack",     20.0f},
                {"compRelease",    1000.0f},
                {"compMakeup",     2.0f},
                {"compMix",        75.0f},
                {"eqModel",        3.0f},  // Pultec
                {"eqLowGain",      3.0f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -1.0f},
                {"eqMidFreq",      400.0f},
                {"eqHighGain",     2.0f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        addFactory("Deep Groove",
            "Deep House",
            "Groove-focused deep house. "
            "SSL compression locks the groove. "
            "Neve warmth underneath.",
            {
                {"satModel",       0.0f},  // NEVE
                {"drive",          20.0f},
                {"satMix",         80.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -16.0f},
                {"compRatio",      4.0f},
                {"compAttack",     1.0f},
                {"compRelease",    200.0f},
                {"compMakeup",     3.0f},
                {"compMix",        80.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      2.5f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -2.0f},
                {"eqMidFreq",      350.0f},
                {"eqHighGain",     1.5f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -0.5f}
            });

        addFactory("Late Night Deep",
            "Deep House",
            "Late night intimate deep house. "
            "Subtle tube warmth, "
            "optical smoothing, Pultec air.",
            {
                {"satModel",       3.0f},  // TUBE
                {"drive",          12.0f},
                {"satMix",         70.0f},
                {"compModel",      2.0f},  // LA-2A
                {"compThreshold",  -25.0f},
                {"compRatio",      3.0f},
                {"compAttack",     15.0f},
                {"compRelease",    600.0f},
                {"compMakeup",     1.5f},
                {"compMix",        65.0f},
                {"eqModel",        3.0f},  // Pultec
                {"eqLowGain",      2.0f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -0.5f},
                {"eqMidFreq",      500.0f},
                {"eqHighGain",     3.5f},
                {"eqHighFreq",     16000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        // ── PROGRESSIVE HOUSE EXTENDED ────────
        addFactory("Progressive Peak",
            "Progressive House",
            "Festival-ready progressive peak time. "
            "Maximum energy and impact. "
            "API punch with SSL control.",
            {
                {"satModel",       2.0f},  // API
                {"drive",          35.0f},
                {"satMix",         80.0f},
                {"compModel",      4.0f},  // API 2500
                {"compThreshold",  -14.0f},
                {"compRatio",      6.0f},
                {"compAttack",     0.3f},
                {"compRelease",    100.0f},
                {"compMakeup",     4.0f},
                {"compMix",        85.0f},
                {"eqModel",        4.0f},  // API 550A
                {"eqLowGain",      3.5f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -2.0f},
                {"eqMidFreq",      500.0f},
                {"eqHighGain",     3.5f},
                {"eqHighFreq",     10000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -1.5f}
            });

        addFactory("Progressive Atmosphere",
            "Progressive House",
            "Atmospheric progressive breakdown. "
            "Wide, lush, emotional. "
            "Fairchild glue with Neve air.",
            {
                {"satModel",       4.0f},  // TAPE
                {"drive",          15.0f},
                {"satMix",         75.0f},
                {"compModel",      1.0f},  // Fairchild
                {"compThreshold",  -22.0f},
                {"compRatio",      3.0f},
                {"compAttack",     30.0f},
                {"compRelease",    1500.0f},
                {"compMakeup",     2.0f},
                {"compMix",        60.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      1.5f},
                {"eqLowFreq",      110.0f},
                {"eqMidGain",      -1.0f},
                {"eqMidFreq",      400.0f},
                {"eqHighGain",     3.5f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        // ── MELODIC HOUSE EXTENDED ────────────
        addFactory("Melodic Emotion",
            "Melodic House",
            "Emotional melodic house. "
            "Tube warmth for feeling. "
            "LA-2A transparency. Neve presence.",
            {
                {"satModel",       3.0f},  // TUBE
                {"drive",          18.0f},
                {"satMix",         85.0f},
                {"compModel",      2.0f},  // LA-2A
                {"compThreshold",  -20.0f},
                {"compRatio",      3.0f},
                {"compAttack",     12.0f},
                {"compRelease",    400.0f},
                {"compMakeup",     3.0f},
                {"compMix",        90.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      2.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      1.5f},
                {"eqMidFreq",      2000.0f},
                {"eqHighGain",     3.0f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        addFactory("Melodic Tension",
            "Melodic House",
            "Building tension in melodic house. "
            "FET edge and energy. "
            "Forward API mids cutting through.",
            {
                {"satModel",       5.0f},  // FET
                {"drive",          28.0f},
                {"satMix",         75.0f},
                {"compModel",      3.0f},  // 1176
                {"compThreshold",  -14.0f},
                {"compRatio",      4.0f},
                {"compAttack",     3.0f},
                {"compRelease",    100.0f},
                {"compMakeup",     4.0f},
                {"compMix",        80.0f},
                {"eqModel",        4.0f},  // API 550A
                {"eqLowGain",      2.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      2.5f},
                {"eqMidFreq",      2500.0f},
                {"eqHighGain",     2.5f},
                {"eqHighFreq",     10000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        // ── TECHNO ────────────────────────────
        addFactory("Dark Techno",
            "Techno",
            "Dark industrial techno. "
            "Aggressive FET saturation. "
            "1176 all-in. Heavy and relentless.",
            {
                {"satModel",       5.0f},  // FET
                {"drive",          55.0f},
                {"satMix",         70.0f},
                {"compModel",      3.0f},  // 1176
                {"compThreshold",  -10.0f},
                {"compRatio",      12.0f},
                {"compAttack",     0.5f},
                {"compRelease",    50.0f},
                {"compMakeup",     5.0f},
                {"compMix",        55.0f},
                {"eqModel",        4.0f},  // API 550A
                {"eqLowGain",      2.0f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -3.0f},
                {"eqMidFreq",      400.0f},
                {"eqHighGain",     1.5f},
                {"eqHighFreq",     8000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -2.0f}
            });

        addFactory("Techno Kick",
            "Techno",
            "Punishing techno kick. "
            "Maximum punch and attack. "
            "API smash with SSL control.",
            {
                {"satModel",       2.0f},  // API
                {"drive",          45.0f},
                {"satMix",         70.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -10.0f},
                {"compRatio",      8.0f},
                {"compAttack",     0.1f},
                {"compRelease",    60.0f},
                {"compMakeup",     6.0f},
                {"compMix",        75.0f},
                {"eqModel",        4.0f},  // API 550A
                {"eqLowGain",      5.0f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -3.0f},
                {"eqMidFreq",      400.0f},
                {"eqHighGain",     2.0f},
                {"eqHighFreq",     8000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -2.0f}
            });

        addFactory("Techno Bus",
            "Techno",
            "Techno mix bus glue. "
            "SSL tight control with tape cohesion. "
            "SSL clean EQ.",
            {
                {"satModel",       4.0f},  // TAPE
                {"drive",          18.0f},
                {"satMix",         75.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -16.0f},
                {"compRatio",      4.0f},
                {"compAttack",     1.0f},
                {"compRelease",    150.0f},
                {"compMakeup",     2.5f},
                {"compMix",        80.0f},
                {"eqModel",        2.0f},  // SSL 4000E
                {"eqLowGain",      2.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -2.0f},
                {"eqMidFreq",      500.0f},
                {"eqHighGain",     2.0f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -1.5f}
            });

        // ── AMBIENT ───────────────────────────
        addFactory("Ambient Pad",
            "Ambient",
            "Infinite ambient texture. "
            "Iron transformer color only. "
            "Fairchild TC6 program auto. "
            "Pultec vintage air.",
            {
                {"satModel",       6.0f},  // IRON
                {"drive",          8.0f},
                {"satMix",         80.0f},
                {"compModel",      1.0f},  // Fairchild
                {"compThreshold",  -28.0f},
                {"compRatio",      3.0f},
                {"compAttack",     50.0f},
                {"compRelease",    2000.0f},
                {"compMakeup",     1.0f},
                {"compMix",        50.0f},
                {"eqModel",        3.0f},  // Pultec
                {"eqLowGain",      1.5f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -0.5f},
                {"eqMidFreq",      500.0f},
                {"eqHighGain",     4.0f},
                {"eqHighFreq",     16000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        addFactory("Ambient Drone",
            "Ambient",
            "Dark evolving drone. "
            "Tape saturation for texture. "
            "Optical smooth control. "
            "Subtle low presence.",
            {
                {"satModel",       4.0f},  // TAPE
                {"drive",          25.0f},
                {"satMix",         80.0f},
                {"compModel",      2.0f},  // LA-2A
                {"compThreshold",  -20.0f},
                {"compRatio",      3.0f},
                {"compAttack",     15.0f},
                {"compRelease",    2000.0f},
                {"compMakeup",     2.0f},
                {"compMix",        85.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      3.0f},
                {"eqLowFreq",      110.0f},
                {"eqMidGain",      -2.0f},
                {"eqMidFreq",      600.0f},
                {"eqHighGain",     -1.0f},
                {"eqHighFreq",     10000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        // ── LIVE PERFORMANCE EXTENDED ─────────
        addFactory("Live Warmup",
            "Live Performance",
            "Early set warm and inviting. "
            "Gentle tube color. "
            "Optical control. Neve tone.",
            {
                {"satModel",       3.0f},  // TUBE
                {"drive",          10.0f},
                {"satMix",         65.0f},
                {"compModel",      2.0f},  // LA-2A
                {"compThreshold",  -24.0f},
                {"compRatio",      3.0f},
                {"compAttack",     15.0f},
                {"compRelease",    600.0f},
                {"compMakeup",     1.5f},
                {"compMix",        55.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      2.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -0.5f},
                {"eqMidFreq",      400.0f},
                {"eqHighGain",     2.0f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        addFactory("Live Build",
            "Live Performance",
            "Building energy through the set. "
            "API punch increasing. "
            "SSL control tightening.",
            {
                {"satModel",       2.0f},  // API
                {"drive",          25.0f},
                {"satMix",         75.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -18.0f},
                {"compRatio",      4.0f},
                {"compAttack",     2.0f},
                {"compRelease",    200.0f},
                {"compMakeup",     2.0f},
                {"compMix",        75.0f},
                {"eqModel",        2.0f},  // SSL 4000E
                {"eqLowGain",      2.5f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -1.5f},
                {"eqMidFreq",      500.0f},
                {"eqHighGain",     2.5f},
                {"eqHighFreq",     12000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -1.0f}
            });

        addFactory("Live After Hours",
            "Live Performance",
            "Post-peak intimate after hours. "
            "Warm and deep. "
            "Pultec vintage character throughout.",
            {
                {"satModel",       0.0f},  // NEVE
                {"drive",          15.0f},
                {"satMix",         80.0f},
                {"compModel",      1.0f},  // Fairchild
                {"compThreshold",  -22.0f},
                {"compRatio",      3.0f},
                {"compAttack",     20.0f},
                {"compRelease",    800.0f},
                {"compMakeup",     2.0f},
                {"compMix",        65.0f},
                {"eqModel",        3.0f},  // Pultec
                {"eqLowGain",      2.5f},
                {"eqLowFreq",      60.0f},
                {"eqMidGain",      -0.5f},
                {"eqMidFreq",      400.0f},
                {"eqHighGain",     2.5f},
                {"eqHighFreq",     16000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        // ── MIXING TOOLS ──────────────────────
        addFactory("Mix Reference",
            "Mixing",
            "Transparent mix reference chain. "
            "Minimal coloration. "
            "Just enough control to polish.",
            {
                {"satModel",       6.0f},  // IRON
                {"drive",          5.0f},
                {"satMix",         50.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -24.0f},
                {"compRatio",      1.5f},
                {"compAttack",     10.0f},
                {"compRelease",    500.0f},
                {"compMakeup",     0.5f},
                {"compMix",        40.0f},
                {"eqModel",        2.0f},  // SSL 4000E
                {"eqLowGain",      0.5f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      0.0f},
                {"eqMidFreq",      1000.0f},
                {"eqHighGain",     0.5f},
                {"eqHighFreq",     16000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });

        addFactory("Parallel Drums",
            "Mixing",
            "New York parallel compression. "
            "Send drums to this. "
            "Blend for density and punch.",
            {
                {"satModel",       5.0f},  // FET
                {"drive",          40.0f},
                {"satMix",         80.0f},
                {"compModel",      3.0f},  // 1176
                {"compThreshold",  -6.0f},
                {"compRatio",      20.0f},
                {"compAttack",     0.5f},
                {"compRelease",    40.0f},
                {"compMakeup",     8.0f},
                {"compMix",        30.0f},
                {"eqModel",        4.0f},  // API 550A
                {"eqLowGain",      4.0f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      2.0f},
                {"eqMidFreq",      3000.0f},
                {"eqHighGain",     3.0f},
                {"eqHighFreq",     8000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     -3.0f}
            });

        addFactory("Synth Bus",
            "Mixing",
            "Electronic synth bus glue. "
            "SSL control with Neve warmth. "
            "Keeps synths together.",
            {
                {"satModel",       1.0f},  // SSL
                {"drive",          12.0f},
                {"satMix",         70.0f},
                {"compModel",      0.0f},  // SSL Bus
                {"compThreshold",  -20.0f},
                {"compRatio",      3.0f},
                {"compAttack",     3.0f},
                {"compRelease",    300.0f},
                {"compMakeup",     2.0f},
                {"compMix",        70.0f},
                {"eqModel",        0.0f},  // Neve 1073
                {"eqLowGain",      1.5f},
                {"eqLowFreq",      80.0f},
                {"eqMidGain",      -1.0f},
                {"eqMidFreq",      400.0f},
                {"eqHighGain",     2.0f},
                {"eqHighFreq",     10000.0f},
                {"inputGain",      0.0f},
                {"outputGain",     0.0f}
            });
    }

    //──────────────────────────────────────────
    // HELPER - Build preset from value map
    //──────────────────────────────────────────
    void addFactory(
        const juce::String& name,
        const juce::String& category,
        const juce::String& description,
        std::initializer_list<
            std::pair<const char*, float>> values)
    {
        ModulatedStripPreset preset;
        preset.name        = name;
        preset.category    = category;
        preset.description = description;
        preset.author      = "Modulated Strip";

        // Build XML state
        auto state = apvts.copyState();

        // Set each parameter value
        for (auto& kv : values)
        {
            if (auto* param = apvts
                .getParameter(kv.first))
            {
                float normalised =
                    param->convertTo0to1(kv.second);
                param->setValueNotifyingHost(normalised);
            }
        }

        // Capture state
        auto newState = apvts.copyState();
        preset.state = newState.createXml();

        // Restore original state
        apvts.replaceState(state);

        factoryPresets.push_back(std::move(preset));
    }
};