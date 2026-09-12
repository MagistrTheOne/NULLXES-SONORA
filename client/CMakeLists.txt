cmake_minimum_required(VERSION 3.22)

project(sonora-runtime VERSION 0.1.0 LANGUAGES CXX)

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
    VERSION 0.1.0
    NEEDS_WEB_BROWSER FALSE
)

juce_generate_juce_header(SONORA)

target_sources(SONORA PRIVATE
    src/app/Application.cpp
    src/app/MainWindow.h
    src/app/MainWindow.cpp
    src/state/AppState.h
    src/state/AppState.cpp
    src/models/AudioAnalysis.h
    src/models/Issue.h
    src/models/Insight.h
    src/ui/theme/Colors.h
    src/ui/theme/Typography.h
    src/ui/theme/Theme.h
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
    src/ui/components/AssistantPanel.h
    src/ui/components/AssistantPanel.cpp
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
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
