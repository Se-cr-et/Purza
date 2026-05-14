cmake_minimum_required(VERSION 3.20)
project(Purza)

set(CMAKE_CXX_STANDARD 17)

# Ensure headers in 'include' are visible to all files
include_directories(include)

# --- 1. CORE LOGIC & DATABASE ---
# Include the parser and the VectorStore implementation
set(PURZA_LOGIC
        src/parser.cpp
        # src/FlatVectorStore.cpp  # Uncomment this if you moved DB logic to a .cpp
)

# --- 2. THE SERVER APPLICATION ---
# This builds the main background process (the engine)
add_executable(Purza
        src/server.cpp
        ${PURZA_LOGIC}
        src/Parser.h
)

# Crucial: Link pthread for multi-threading support
find_package(Threads REQUIRED)
target_link_libraries(Purza PRIVATE Threads::Threads)

# --- 3. THE CLIENT APPLICATION ---
# You need a separate executable for the CLI to talk to the server
add_executable(vdb_cli
        src/client.cpp
)

# --- 4. THE TEST EXECUTABLE ---
add_executable(Purza_Test
        test/main.cpp
        ${PURZA_LOGIC}
)
target_link_libraries(Purza_Test PRIVATE Threads::Threads)