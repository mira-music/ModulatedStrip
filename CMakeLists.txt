cmake_minimum_required(VERSION 3.22)

project(ModulatedStrip VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

# Point to JUCE
add_subdirectory(C:/Projects/JUCE JUCE)

# Create the plugin
juce_add_plugin(ModulatedStrip
    COMPANY_NAME "YourName"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Mdst
    PLUGIN_CODE Mstr
    FORMATS VST3 Standalone
    PRODUCT_NAME "Modulated Strip"
)

# Our source files
target_sources(ModulatedStrip PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

# Header files (for IDE visibility)
target_sources(ModulatedStrip PRIVATE
    Source/PluginProcessor.h
    Source/PluginEditor.h
    Source/AnalogMath.h
    Source/SaturationProcessor.h
    Source/CompressorProcessor.h
    Source/EQProcessor.h
)

# JUCE settings
target_compile_definitions(ModulatedStrip PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link JUCE modules we need
target_link_libraries(ModulatedStrip PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# Generate JuceHeader.h
juce_generate_juce_header(ModulatedStrip)