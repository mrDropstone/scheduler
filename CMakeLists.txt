cmake_minimum_required(VERSION 3.15)
project(scheduler LANGUAGES CXX)

# Point CMake to the root folder of your extracted OR-Tools archive.
# Replace this path with the actual location where you extracted it.
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/or-tools")
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json SYSTEM INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/json/include"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This finds the OR-Tools package and creates the ortools::ortools target
find_package(ortools CONFIG REQUIRED)

add_executable(scheduler src/main.cpp src/csv.cpp src/scheduler.cpp)

# Link your executable against OR-Tools
target_link_libraries(scheduler PRIVATE ortools::ortools PRIVATE nlohmann_json)
