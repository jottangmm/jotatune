cmake_minimum_required(VERSION 3.22)
project(JotaTune VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(JotaTune
    COMPANY_NAME "Jota"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Jota
    PLUGIN_CODE Jtun
    FORMATS VST3
    COPY_PLUGIN_AFTER_BUILD FALSE
    PRODUCT_NAME "JotaTune")

juce_generate_juce_header(JotaTune)

target_sources(JotaTune PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp)

target_compile_definitions(JotaTune PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(JotaTune
    PRIVATE juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_warning_flags)
