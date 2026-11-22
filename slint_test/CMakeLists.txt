cmake_minimum_required(VERSION 3.21)

project(test LANGUAGES CXX)

SET(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    Slint
    GIT_REPOSITORY https://github.com/slint-ui/slint.git
    GIT_TAG release/1
    SOURCE_SUBDIR api/cpp
)
FetchContent_MakeAvailable(Slint)


add_executable(test main.cpp)

slint_target_sources(test ui/main.slint)

target_link_libraries(test PRIVATE Slint::Slint)
