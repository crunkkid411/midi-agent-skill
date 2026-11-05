# MIDI Agent - a VST3 instrument for Cubase that hosts the midi-agent-skill
# composition engine with a piano roll per segment.
#
# Build (Windows, VS 2022 Build Tools + CMake):
#   cmake -S vst3/midi-agent -B build/midi-agent -G "Visual Studio 17 2022" -A x64 ^
#         -DSMTG_VST3_SDK_DIR=X:/AI-2/Steinberg-VST-factory/vst3sdk
#   cmake --build build/midi-agent --config Release
#
# Build (Linux, for compile checks and the core tests):
#   cmake -S vst3/midi-agent -B build/midi-agent -DSMTG_VST3_SDK_DIR=/path/to/vst3sdk
#   cmake --build build/midi-agent -j
#
# See docs/BUILD.md for the full procedure.

cmake_minimum_required(VERSION 3.25.0)

project(MidiAgent
    VERSION 0.1.0.0
    DESCRIPTION "MIDI Agent VST3 - midi-agent-skill inside Cubase"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MIDIAGENT_BUILD_PLUGIN "Build the VST3 plug-in (needs the VST3 SDK)" ON)
option(MIDIAGENT_BUILD_TESTS "Build the core unit tests" ON)

# ---------------------------------------------------------------------------
# core: host independent library (no SDK, no VSTGUI). This is the part the
# unit tests cover and the part a future agent can change without a DAW.
# ---------------------------------------------------------------------------
set(midiagent_core_sources
    core/json.h
    core/json.cpp
    core/composition.h
    core/composition.cpp
    core/gm_instruments.h
    core/gm_instruments.cpp
    core/segments.h
    core/segments.cpp
    core/midifile.h
    core/midifile.cpp
    core/wavfile.h
    core/wavfile.cpp
    core/synth.h
    core/synth.cpp
    core/sequencer.h
    core/sequencer.cpp
    core/renderer.h
    core/renderer.cpp
    core/agent_bridge.h
    core/agent_bridge.cpp
    core/platform.h
    core/platform.cpp
)

add_library(midiagent_core STATIC ${midiagent_core_sources})
target_include_directories(midiagent_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(midiagent_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
    target_compile_options(midiagent_core PRIVATE /W4 /utf-8)
    target_compile_definitions(midiagent_core PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(midiagent_core PRIVATE -Wall -Wextra)
endif()

# ---------------------------------------------------------------------------
# tests: a tiny self-contained runner, no framework dependency.
# ---------------------------------------------------------------------------
if(MIDIAGENT_BUILD_TESTS)
    enable_testing()
    add_executable(midiagent_tests
        tests/test_main.cpp
        tests/test_json.cpp
        tests/test_composition.cpp
        tests/test_segments.cpp
        tests/test_midifile.cpp
        tests/test_wavfile.cpp
        tests/test_synth.cpp
        tests/test_sequencer.cpp
        tests/test_agent_bridge.cpp
    )
    target_link_libraries(midiagent_tests PRIVATE midiagent_core)
    target_compile_definitions(midiagent_tests PRIVATE
        MIDIAGENT_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
    add_test(NAME midiagent_tests COMMAND midiagent_tests)
endif()

# ---------------------------------------------------------------------------
# plugin: the VST3 module. Follows the layout of Steinberg's VST3 Project
# Generator output (source/, resource/, add_subdirectory of the SDK).
# ---------------------------------------------------------------------------
if(MIDIAGENT_BUILD_PLUGIN)
    set(SMTG_VST3_SDK_DIR "" CACHE PATH "Path to the VST3 SDK root (the folder with pluginterfaces/, public.sdk/, vstgui4/)")
    if(NOT SMTG_VST3_SDK_DIR)
        message(FATAL_ERROR "SMTG_VST3_SDK_DIR is empty. Pass -DSMTG_VST3_SDK_DIR=<vst3sdk root>, or -DMIDIAGENT_BUILD_PLUGIN=OFF for core only.")
    endif()
    set(vst3sdk_SOURCE_DIR "${SMTG_VST3_SDK_DIR}")

    # The SDK examples are not needed and slow the build down.
    set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SMTG_ENABLE_VSTGUI_SUPPORT ON CACHE BOOL "" FORCE)

    set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")
    add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
    smtg_enable_vst3_sdk()

    set(midiagent_plugin_sources
        source/version.h
        source/plugids.h
        source/messages.h
        source/session.h
        source/processor.h
        source/processor.cpp
        source/controller.h
        source/controller.cpp
        source/editor.h
        source/editor.cpp
        source/factory.cpp
        source/ui/theme.h
        source/ui/theme.cpp
        source/ui/widgets.h
        source/ui/widgets.cpp
        source/ui/pianoroll_view.h
        source/ui/pianoroll_view.cpp
        source/ui/segment_strip_view.h
        source/ui/segment_strip_view.cpp
        source/ui/track_list_view.h
        source/ui/track_list_view.cpp
        source/ui/header_view.h
        source/ui/header_view.cpp
    )

    smtg_add_vst3plugin(MidiAgent
        PACKAGE_NAME "MIDI Agent"
        SOURCES_LIST ${midiagent_plugin_sources}
    )

    target_link_libraries(MidiAgent PRIVATE midiagent_core sdk vstgui_support)
    target_include_directories(MidiAgent PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    smtg_target_configure_version_file(MidiAgent)

    if(SMTG_WIN)
        target_sources(MidiAgent PRIVATE resource/win32resource.rc)
        target_compile_definitions(MidiAgent PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
    endif()
    # Headless host smoke test: loads the built plug-in like a DAW would.
    if(MIDIAGENT_BUILD_TESTS)
        set(midiagent_host_smoke_sources
            tests/host_smoke.cpp
            ${SMTG_VST3_SDK_DIR}/public.sdk/source/common/memorystream.cpp
            ${SMTG_VST3_SDK_DIR}/public.sdk/source/vst/hosting/plugprovider.cpp
        )
        if(SMTG_MAC)
            list(APPEND midiagent_host_smoke_sources ${SMTG_VST3_SDK_DIR}/public.sdk/source/vst/hosting/module_mac.mm)
        elseif(SMTG_LINUX)
            list(APPEND midiagent_host_smoke_sources ${SMTG_VST3_SDK_DIR}/public.sdk/source/vst/hosting/module_linux.cpp)
        elseif(SMTG_WIN)
            list(APPEND midiagent_host_smoke_sources ${SMTG_VST3_SDK_DIR}/public.sdk/source/vst/hosting/module_win32.cpp)
        endif()
        add_executable(midiagent_host_smoke ${midiagent_host_smoke_sources})
        target_include_directories(midiagent_host_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_link_libraries(midiagent_host_smoke PRIVATE midiagent_core sdk_hosting)
        if(SMTG_LINUX)
            target_link_libraries(midiagent_host_smoke PRIVATE dl pthread)
        endif()
        add_dependencies(midiagent_host_smoke MidiAgent)
        add_test(NAME midiagent_host_smoke
            COMMAND midiagent_host_smoke "$<TARGET_PROPERTY:MidiAgent,SMTG_PLUGIN_PACKAGE_PATH>"
                    "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/scenecore-8bar.composition.json")
    endif()

    if(SMTG_MAC)
        smtg_target_set_bundle(MidiAgent
            BUNDLE_IDENTIFIER "com.midiagentskill.midiagent"
            COMPANY_NAME "midi-agent-skill")
        smtg_target_set_debug_executable(MidiAgent
            "/Applications/VST3PluginTestHost.app"
            "--pluginfolder;$(BUILT_PRODUCTS_DIR)")
    endif()
endif()
