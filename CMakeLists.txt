cmake_minimum_required(VERSION 3.20)
project(Purza)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)

# 1. THE MAIN APPLICATION
# This builds the actual program using files in src
file(GLOB_RECURSE APP_SOURCES "src/*.cpp")
add_executable(Purza ${APP_SOURCES})

# 2. THE TEST EXECUTABLE
# This builds a separate program specifically for testing
file(GLOB_RECURSE TEST_SOURCES "test/main.cpp")
# We usually link the src files to the test too, so the test can actually
# call functions defined in your src folder.
add_executable(Purza_Test ${TEST_SOURCES} ${APP_SOURCES})