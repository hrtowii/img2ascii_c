cmake_minimum_required(VERSION 4.1)
project(img2ascii VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "-Ofast")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -fsanitize=address -fsanitize=undefined")
set(CMAKE_C_FLAGS_RELEASE "-Ofast")
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

option(IMG2ASCII_BUILD_LIB "Build the static library" ON)
option(IMG2ASCII_BUILD_CLI "Build the CLI tool" ON)

find_library(MATH_LIBRARY m)

if(IMG2ASCII_BUILD_LIB)
    add_library(img2ascii STATIC lib/img2ascii.c)
    target_include_directories(img2ascii PUBLIC ${CMAKE_SOURCE_DIR}/lib)
endif()

if(IMG2ASCII_BUILD_CLI)
    option(ARGPARSE_SHARED "Build shared library" OFF)
    option(ARGPARSE_STATIC "Build static library" ON)

    include(FetchContent)
    FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/cofyc/argparse.git
        GIT_TAG        v1.1.0
    )
    FetchContent_MakeAvailable(argparse)

    file(GLOB_RECURSE CLI_SOURCES "src/*.c")
    add_executable(img2ascii_cli ${CLI_SOURCES})
    target_include_directories(img2ascii_cli PRIVATE ${CMAKE_SOURCE_DIR}/lib)
    target_link_libraries(img2ascii_cli PRIVATE img2ascii argparse ${MATH_LIBRARY})
    set_target_properties(img2ascii_cli PROPERTIES
        OUTPUT_NAME "img2ascii"
    )
endif()
