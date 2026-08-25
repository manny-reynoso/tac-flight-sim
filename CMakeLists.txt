cmake_minimum_required(VERSION 3.12)
project(tac-sim-flight CXX)

# Force C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com
  GIT_TAG        5.5 # Or use a specific tag like "5.0"
)

# Make raylib targets available 
FetchContent_MakeAvailable(raylib)

    target_compile_options(tac-sim-flight
        -Wall          # Enable most warning messages
        -Wextra        # Enable extra warning messages
        -Wpedantic     # Enforce strict ISO C++ compliance
        -Wshadow       # Warn when a variable shadows another
        -Wnon-virtual-dtor # Warn when a class has virtual functions but no virtual destructor
    )



# Define your game executable
add_executable(tac-sim-flight src/main.cpp)

# Link raylib to your executable
target_link_libraries(tac-sim-flight raylib)
