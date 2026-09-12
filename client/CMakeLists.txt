cmake_minimum_required(VERSION 3.22)

project(sonora-runtime VERSION 0.3.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Prefer a local JUCE tree. FetchContent + paths with spaces breaks on Windows.
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

juce_add_gui_app(SONORA
    PRODUCT_NAME "SONORA"
    COMPANY_NAME "NULLXES"
    BUNDLE_ID com.nullxes.sonora
    VERSION 0.3.1
    NEEDS_WEB_BROWSER FALSE
)

juce_generate_juce_header(SONORA)

target_sources(SONORA PRIVATE
    src/app/Application.cpp
    src/app/MainWindow.h
    src/app/MainWindow.cpp
    src/state/AppState.h
    src/state/AppState.cpp
    src/backend/ClientLog.h
    src/backend/Fault.h
    src/backend/HttpTransport.h
    src/backend/HttpTransport.cpp
    src/backend/Dto.h
    src/backend/Dto.cpp
    src/backend/ApiClient.h
    src/backend/ApiClient.cpp
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

target_include_directories(SONORA PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(SONORA PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:SONORA,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:SONORA,JUCE_VERSION>"
)

target_link_libraries(SONORA PRIVATE
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

if(WIN32)
    target_link_libraries(SONORA PRIVATE winhttp)
endif()
