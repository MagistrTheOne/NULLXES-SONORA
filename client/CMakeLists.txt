cmake_minimum_required(VERSION 3.22)

project(sonora-runtime VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SONORA_JUCE_DIR "$ENV{SONORA_JUCE_DIR}" CACHE PATH "JUCE 8 source tree")
if(NOT SONORA_JUCE_DIR)
    if(EXISTS "D:/NULLXES/_cache/JUCE-8.0.8/CMakeLists.txt")
        set(SONORA_JUCE_DIR "D:/NULLXES/_cache/JUCE-8.0.8")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/JUCE/CMakeLists.txt")
        set(SONORA_JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/JUCE")
    endif()
endif()

if(SONORA_JUCE_DIR AND EXISTS "${SONORA_JUCE_DIR}/CMakeLists.txt")
    add_subdirectory("${SONORA_JUCE_DIR}" "${CMAKE_BINARY_DIR}/juce")
else()
    include(FetchContent)
    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.8
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(juce)
endif()

set(SONORA_SHARED_SOURCES
    src/state/AppState.h
    src/state/AppState.cpp
    src/backend/ClientLog.h
    src/backend/Fault.h
    src/engine/Engine.h
    src/engine/Engine.cpp
    src/audio/AudioPlayer.h
    src/audio/AudioPlayer.cpp
    src/models/AudioAnalysis.h
    src/models/TrackDna.h
    src/models/Issue.h
    src/models/Insight.h
    src/ui/theme/Colors.h
    src/ui/theme/Typography.h
    src/ui/theme/Theme.h
    src/ui/copy/HumanCopy.h
    src/ui/copy/HumanCopy.cpp
    src/ui/components/ActionButton.h
    src/ui/components/ActionButton.cpp
    src/ui/components/MetricCard.h
    src/ui/components/MetricCard.cpp
    src/ui/components/IssueRow.h
    src/ui/components/IssueRow.cpp
    src/ui/components/SpectrumView.h
    src/ui/components/SpectrumView.cpp
    src/ui/components/InsightPanel.h
    src/ui/components/InsightPanel.cpp
    src/ui/components/StatusRail.h
    src/ui/components/StatusRail.cpp
    src/ui/components/TopBar.h
    src/ui/components/TopBar.cpp
    src/ui/components/ContextRail.h
    src/ui/components/ContextRail.cpp
    src/ui/components/CreatePanel.h
    src/ui/components/CreatePanel.cpp
    src/ui/components/CreatePage.h
    src/ui/components/CreatePage.cpp
    src/ui/components/ProjectCanvas.h
    src/ui/components/ProjectCanvas.cpp
    src/ui/components/DnaView.h
    src/ui/components/DnaView.cpp
    src/ui/components/StructureView.h
    src/ui/components/StructureView.cpp
    src/ui/components/WaveformStrip.h
    src/ui/components/WaveformStrip.cpp
    src/ui/components/ReferencePage.h
    src/ui/components/ReferencePage.cpp
    src/ui/components/IdentityRow.h
    src/ui/components/IdentityRow.cpp
    src/ui/components/HealthGauge.h
    src/ui/components/HealthGauge.cpp
    src/ui/components/ArrangementStrip.h
    src/ui/components/ArrangementStrip.cpp
    src/ui/components/MixHealthView.h
    src/ui/components/MixHealthView.cpp
    src/ui/components/TranslationStrip.h
    src/ui/components/TranslationStrip.cpp
    src/ui/components/MaskingHeatmap.h
    src/ui/components/MaskingHeatmap.cpp
    src/ui/screens/Dashboard.h
    src/ui/screens/Dashboard.cpp
)

set(SONORA_JUCE_LIBS
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

juce_add_gui_app(SONORA
    PRODUCT_NAME "SONORA"
    COMPANY_NAME "NULLXES"
    BUNDLE_ID com.nullxes.sonora
    VERSION 1.0.0
    NEEDS_WEB_BROWSER FALSE
)

juce_generate_juce_header(SONORA)

target_sources(SONORA PRIVATE
    src/app/Application.cpp
    src/app/MainWindow.h
    src/app/MainWindow.cpp
    ${SONORA_SHARED_SOURCES}
)

target_include_directories(SONORA PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(SONORA PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_USE_FLAC=1
    JUCE_USE_MP3AUDIOFORMAT=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    _CRT_SECURE_NO_WARNINGS
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:SONORA,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:SONORA,JUCE_VERSION>"
)

target_link_libraries(SONORA PRIVATE ${SONORA_JUCE_LIBS})

juce_add_plugin(SONORA_VST3
    PRODUCT_NAME "SONORA"
    COMPANY_NAME "NULLXES"
    BUNDLE_ID com.nullxes.sonora.vst3
    PLUGIN_MANUFACTURER_CODE Nlxs
    PLUGIN_CODE Sonr
    FORMATS VST3
    VERSION 1.0.0
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Fx Analyzer"
    DESCRIPTION "Adaptive Sound Intelligence"
)

juce_generate_juce_header(SONORA_VST3)

target_sources(SONORA_VST3 PRIVATE
    ${SONORA_SHARED_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../plugin/PluginProcessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../plugin/PluginProcessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../plugin/PluginEditor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../plugin/PluginEditor.cpp
)

target_include_directories(SONORA_VST3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../plugin
)

target_compile_definitions(SONORA_VST3 PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_USE_FLAC=1
    JUCE_USE_MP3AUDIOFORMAT=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    SONORA_IS_PLUGIN=1
    _CRT_SECURE_NO_WARNINGS
    JUCE_APPLICATION_NAME_STRING="SONORA"
    JUCE_APPLICATION_VERSION_STRING="1.0.0"
)

target_link_libraries(SONORA_VST3 PRIVATE
    ${SONORA_JUCE_LIBS}
    juce::juce_audio_processors
)
