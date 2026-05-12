cmake_minimum_required(VERSION 3.20)
project(Purza)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)

# --- 1. CORE LOGIC (No main functions here) ---
set(PURZA_LOGIC
        src/parser.cpp
        # src/FlatVectorStore.cpp  <-- Add this if it exists
)

# --- 2. THE MAIN APPLICATIONS ---
# Each of these has its own main()
add_executable(Purza src/main.cpp ${PURZA_LOGIC})

# --- 3. THE TEST EXECUTABLE ---
# Link the test file + the core logic only
add_executable(Purza_Test
        test/main.cpp
        ${PURZA_LOGIC}
)