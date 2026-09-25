cmake_minimum_required(VERSION 3.16)

project(Hestia VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# -----------------------------
# whisper.cpp
# -----------------------------
set(WHISPER_LIB_PATH "${PROJECT_SOURCE_DIR}/lib/libwhisper.a")
if(NOT EXISTS ${WHISPER_LIB_PATH})
    message(FATAL_ERROR "libwhisper.a not found at ${WHISPER_LIB_PATH}")
endif()

# -----------------------------
# CrispEmbed (From Source)
# -----------------------------
set(CRISP_EMBED_ROOT ${CMAKE_SOURCE_DIR}/external/CrispEmbed)

if(NOT EXISTS ${CRISP_EMBED_ROOT}/CMakeLists.txt)
    message(FATAL_ERROR "CrispEmbed source not found at ${CRISP_EMBED_ROOT}. Run ./setup.sh first.")
endif()

add_subdirectory(${CRISP_EMBED_ROOT} EXCLUDE_FROM_ALL)


# -----------------------------
# miniaudio (just downloaded Lmao it's great)
# -----------------------------
set(MINIAUDIO_ROOT ${CMAKE_SOURCE_DIR}/external/miniaudio)

if (NOT EXISTS ${MINIAUDIO_ROOT})
    message(FATAL_ERROR "Miniaudio files not found at ${MINIAUDIO_ROOT}. Either run ./setup.sh first, or just download them and put them in /external/miniaudio.")
endif()

add_library(miniaudio STATIC ${MINIAUDIO_ROOT}/miniaudio.c)
target_include_directories(miniaudio PUBLIC ${MINIAUDIO_ROOT})

# -----------------------------
# nlohmann/json
# -----------------------------
set(JSON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib/json)

if(NOT EXISTS ${JSON_INCLUDE_DIR}/json.hpp)
    message(FATAL_ERROR "nlohmann/json not found at ${JSON_INCLUDE_DIR}/json.hpp")
endif()

# -----------------------------
# Library target
# -----------------------------
file(GLOB_RECURSE HESTIA_SOURCES CONFIGURE_DEPENDS 
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_library(HestiaLib ${HESTIA_SOURCES})

target_include_directories(HestiaLib PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
    ${JSON_INCLUDE_DIR}
    ${CRISP_EMBED_ROOT}/src
    ${MINIAUDIO_ROOT}

)

target_link_libraries(HestiaLib PUBLIC miniaudio)

# -----------------------------
# Executable target
# -----------------------------
add_executable(Hestia main.cpp)

target_include_directories(Hestia PRIVATE
    ${JSON_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${CRISP_EMBED_ROOT}/src
    ${MINIAUDIO_ROOT}

)

target_link_libraries(Hestia PRIVATE 
    HestiaLib
    crispembed
    miniaudio
    ${WHISPER_LIB_PATH}
    pthread
    dl
    stdc++
)

set_target_properties(Hestia PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)