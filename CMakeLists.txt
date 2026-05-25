cmake_minimum_required(VERSION 3.20)
project(Purza VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. FIND DEPENDENCIES
find_package(Threads REQUIRED)

# 2. CORE LIBRARY (The "Engine")
# Adjusted to match the PascalCase filenames in image_77d503.png
# and included src/main.cpp which appears to be your core entry point.
add_library(purza_core STATIC
        src/main.cpp
        lib/init.cpp
        src/Parser.h
        src/FlatVectorStore.h
)

# Added 'src' and 'lib' to include paths so headers are found easily
target_include_directories(purza_core PUBLIC include src lib)
target_link_libraries(purza_core PUBLIC Threads::Threads)

# 3. THE SERVER APPLICATION
# Matches src/server.cpp in image_77d503.png
add_executable(Purza src/server.cpp)
target_link_libraries(Purza PRIVATE purza_core)

# 4. THE CLIENT APPLICATION
# Matches src/client.cpp in image_77d503.png
add_executable(vdb_cli src/client.cpp)
target_link_libraries(vdb_cli PRIVATE purza_core)

# 5. THE TEST EXECUTABLE
# Matches test/main.cpp in image_77d503.png
add_executable(Purza_Test test/main.cpp)
target_link_libraries(Purza_Test PRIVATE purza_core)