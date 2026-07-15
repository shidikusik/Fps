cmake_minimum_required(VERSION 3.16)
project(bloodrush CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Use system raylib when available, otherwise fetch and build it in-tree
# so `mkdir build && cd build && cmake .. && make` works anywhere.
find_package(raylib 5.0 QUIET)
if(NOT raylib_FOUND)
    message(STATUS "System raylib not found — fetching raylib 5.5")
    include(FetchContent)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(raylib)
endif()

add_executable(bloodrush
    src/main.cpp
    src/player.cpp
    src/arena.cpp
)

target_link_libraries(bloodrush PRIVATE raylib)

if(UNIX AND NOT APPLE)
    target_link_libraries(bloodrush PRIVATE m)
endif()
