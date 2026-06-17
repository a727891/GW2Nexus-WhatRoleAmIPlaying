cmake_minimum_required(VERSION 3.20)
project(NexusWhatAmIPlaying LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(NEXUSWHATAMIPLAYING_CROSS_COMPILE "Cross-compile for Windows using MinGW" ON)

if(NEXUSWHATAMIPLAYING_CROSS_COMPILE)
    if(NOT CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/mingw-w64-toolchain.cmake"
            CACHE FILEPATH "MinGW cross-compile toolchain")
    endif()
endif()

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.80
)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    )
    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
endif()

set(SRC_FILES
    src/entry.cpp
    src/core/AppState.cpp
    src/core/SettingsStore.cpp
    src/core/MumbleUtils.cpp
    src/data/RoleConfigJson.cpp
    src/data/RoleModels.cpp
    src/services/HttpClient.cpp
    src/services/RoleConfigLoader.cpp
    src/services/RoleConfigService.cpp
    src/services/RoleSelectionService.cpp
    src/ui/DatAssetIconService.cpp
    src/ui/MainWindow.cpp
    src/ui/QuickAccessService.cpp
    src/ui/OptionsPanel.cpp
)

add_library(NexusWhatAmIPlaying SHARED ${SRC_FILES})

target_include_directories(NexusWhatAmIPlaying PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/extern
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/nexus
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/mumble
)

target_link_libraries(NexusWhatAmIPlaying PRIVATE
    imgui
    nlohmann_json::nlohmann_json
    winhttp
    ws2_32
    crypt32
    shell32
)

target_compile_definitions(NexusWhatAmIPlaying PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

if(CMAKE_CXX_COMPILER MATCHES "mingw" OR MINGW)
    target_link_options(NexusWhatAmIPlaying PRIVATE
        -static-libgcc
        -static-libstdc++
        -static
    )
endif()

set_target_properties(NexusWhatAmIPlaying PROPERTIES
    OUTPUT_NAME "NexusWhatAmIPlaying"
    PREFIX ""
    SUFFIX ".dll"
)
