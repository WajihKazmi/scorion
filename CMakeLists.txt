cmake_minimum_required(VERSION 3.22)

project(Scorion VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SCORION_BUILD_TESTS "Build unit tests" ON)
option(SCORION_BUILD_VST3 "Build VST3" ON)
option(SCORION_BUILD_AU "Build Audio Unit (macOS)" ON)
option(SCORION_BUILD_STANDALONE "Build Standalone" ON)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

set(SCORION_FORMATS "")
if(SCORION_BUILD_STANDALONE)
    list(APPEND SCORION_FORMATS Standalone)
endif()
if(SCORION_BUILD_VST3)
    list(APPEND SCORION_FORMATS VST3)
endif()
if(SCORION_BUILD_AU AND APPLE)
    list(APPEND SCORION_FORMATS AU)
endif()

juce_add_plugin(Scorion
    COMPANY_NAME "Wajih Kazmi"
    BUNDLE_ID com.wajihkazmi.scorion
    PLUGIN_MANUFACTURER_CODE WajK
    PLUGIN_CODE Scor
    FORMATS ${SCORION_FORMATS}
    PRODUCT_NAME "Scorion"
    PLUGIN_NAME "Scorion"
    DESCRIPTION "Scorion flagship virtual instrument"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES Instrument Synth
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
)

juce_add_binary_data(ScorionFonts
    SOURCES
        Resources/fonts/SpaceGrotesk-Bold.ttf
        Resources/fonts/SpaceGrotesk-SemiBold.ttf
        Resources/fonts/Manrope-Regular.ttf
        Resources/fonts/Manrope-Medium.ttf
        Resources/fonts/Manrope-SemiBold.ttf
)

target_sources(Scorion
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/audio/AudioEngine.cpp
        Source/audio/VoiceManager.cpp
        Source/audio/ParamBridge.cpp
        Source/audio/SpscQueue.cpp
        Source/dsp/PolyBlepOscillator.cpp
        Source/dsp/AdsrEnvelope.cpp
        Source/dsp/ZdfFilter.cpp
        Source/dsp/Smoother.cpp
        Source/synth/ISynthEngine.cpp
        Source/synth/VirtualAnalogEngine.cpp
        Source/synth/WavetableEngine.cpp
        Source/synth/FmEngine.cpp
        Source/synth/GranularEngine.cpp
        Source/synth/SamplerEngine.cpp
        Source/modulation/ModulationMatrix.cpp
        Source/modulation/Lfo.cpp
        Source/modulation/MidiLearn.cpp
        Source/fx/FxRack.cpp
        Source/fx/Effects.cpp
        Source/midi/MpeHandler.cpp
        Source/presets/PresetManager.cpp
        Source/assets/AssetLoader.cpp
        Source/assets/ResourcePaths.cpp
        Source/ui/ScorionLookAndFeel.cpp
        Source/ui/WaveformProbe.cpp
)

target_include_directories(Scorion
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_compile_definitions(Scorion
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
)

target_link_libraries(Scorion
    PRIVATE
        ScorionFonts
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
)

# Bundle factory presets + wavetables into every plugin format
function(scorion_copy_resources target_name)
    if(NOT TARGET ${target_name})
        return()
    endif()

    if(APPLE)
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rm -rf
                "$<TARGET_BUNDLE_CONTENT_DIR:${target_name}>/Resources/ScorionResources"
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_BUNDLE_CONTENT_DIR:${target_name}>/Resources/ScorionResources"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/Resources"
                "$<TARGET_BUNDLE_CONTENT_DIR:${target_name}>/Resources/ScorionResources"
            COMMENT "Bundling Scorion Resources into ${target_name}"
        )
    elseif(WIN32)
        # VST3 bundle: <artefact>/Scorion.vst3/Contents/Resources/ScorionResources
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${target_name}>/../Resources/ScorionResources"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/Resources"
                "$<TARGET_FILE_DIR:${target_name}>/../Resources/ScorionResources"
            COMMENT "Bundling Scorion Resources into ${target_name} (Windows)"
        )
    endif()
endfunction()

scorion_copy_resources(Scorion_Standalone)
scorion_copy_resources(Scorion_VST3)
if(APPLE)
    scorion_copy_resources(Scorion_AU)
endif()

if(SCORION_BUILD_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(ScorionTests
        Tests/test_smoother.cpp
        Tests/test_adsr.cpp
        Tests/test_polyblep.cpp
        Tests/test_modulation.cpp
        Tests/test_preset.cpp
        Tests/test_spsc_queue.cpp
        Tests/test_wavetable_bin.cpp
        Tests/test_offline_voice.cpp
        Source/dsp/Smoother.cpp
        Source/dsp/AdsrEnvelope.cpp
        Source/dsp/PolyBlepOscillator.cpp
        Source/dsp/ZdfFilter.cpp
        Source/modulation/ModulationMatrix.cpp
        Source/modulation/Lfo.cpp
        Source/presets/PresetManager.cpp
        Source/audio/SpscQueue.cpp
        Source/synth/ISynthEngine.cpp
        Source/synth/VirtualAnalogEngine.cpp
        Source/synth/WavetableEngine.cpp
        Source/synth/FmEngine.cpp
        Source/synth/GranularEngine.cpp
        Source/synth/SamplerEngine.cpp
        Source/assets/ResourcePaths.cpp
    )

    target_include_directories(ScorionTests PRIVATE Source)
    target_compile_definitions(ScorionTests PRIVATE
        SCORION_UNIT_TEST=1
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        SCORION_REPO_ROOT="${CMAKE_SOURCE_DIR}"
    )
    target_link_libraries(ScorionTests PRIVATE
        Catch2::Catch2WithMain
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
    )

    include(CTest)
    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
    include(Catch)
    catch_discover_tests(ScorionTests)
endif()
