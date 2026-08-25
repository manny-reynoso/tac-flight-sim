cmake_minimum_required(VERSION 3.12)

project(tac-sim-flight LANGUAGES C CXX)

# Configure the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

# Create the executable before configuring it
add_executable(
    tac-flight-sim
    src/main.cpp
)

target_compile_options(
    tac-flight-sim
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
)

target_link_libraries(
    tac-flight-sim
    PRIVATE
        raylib
)
