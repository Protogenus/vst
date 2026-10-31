# CMakeLists for Absolute Reason VST3 (JUCE 8)
cmake_minimum_required(VERSION 3.22)
project(AbsoluteReason VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE from third_party (shallow clone). Override with -DJUCE_DIR=... if needed.
if(NOT DEFINED JUCE_DIR)
    set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/JUCE")
endif()
add_subdirectory(${JUCE_DIR} juce_build)

juce_add_plugin(AbsoluteReason
    COMPANY_NAME "Black River Visuals"
    BUNDLE_ID com.blackrivervisuals.absolutereason
    PLUGIN_MANUFACTURER_CODE BrRv
    PLUGIN_CODE AbRs
    FORMATS VST3
    PRODUCT_NAME "Absolute Reason"
    DESCRIPTION "Growling detuned-saw instrument — reverse-engineered from Stereo Dancer 'Absolute Reason'."
    VERSION 1.0.0
    VST3_CATEGORIES "Instrument" "Synth"
)

target_sources(AbsoluteReason PRIVATE
    juce/PluginProcessor.cpp
    juce/PluginEditor.cpp
)

target_include_directories(AbsoluteReason PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(AbsoluteReason PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(AbsoluteReason PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_basics
)

juce_generate_juce_header(AbsoluteReason)
