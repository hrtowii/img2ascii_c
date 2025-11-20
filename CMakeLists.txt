cmake_minimum_required(VERSION 4.1)
project(img2ascii VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "-O3")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
option(ARGPARSE_SHARED "Build shared library" OFF)
option(ARGPARSE_STATIC "Build static library" ON)

include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/cofyc/argparse.git
    GIT_TAG        v1.1.0
    # or, you could use a commit hash
    #GIT_TAG       15091715cc379bc647d5de6345185a288a35f999
)
FetchContent_MakeAvailable(argparse)

include_directories(lib)

file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(${PROJECT_NAME} PRIVATE argparse)
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "img2ascii"
)
