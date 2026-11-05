cmake_minimum_required(VERSION 3.14)
project(cp_scheduler LANGUAGES CXX)

# Point CMake to the root folder of your extracted OR-Tools archive.
# Replace this path with the actual location where you extracted it.
list(APPEND CMAKE_PREFIX_PATH "/home/lohman/code/projects/cp_scheduler/or-tools")

# This finds the OR-Tools package and creates the ortools::ortools target
find_package(ortools CONFIG REQUIRED)

add_executable(my_app main.cpp csv.cpp scheduler.cpp)

# Link your executable against OR-Tools
target_link_libraries(my_app PRIVATE ortools::ortools)
