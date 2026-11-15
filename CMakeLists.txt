cmake_minimum_required(VERSION 3.21)
project(scheduler LANGUAGES CXX)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/or-tools")
#list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/slint")

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json SYSTEM INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/json/include"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ortools CONFIG REQUIRED)
#find_package(Slint CONFIG REQUIRED)

add_executable(scheduler src/main.cpp src/csv.cpp src/scheduler.cpp)

target_link_libraries(scheduler
    PRIVATE ortools::ortools
    PRIVATE nlohmann_json
    #    PRIVATE Slint::Slint
)
