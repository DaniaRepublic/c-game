cmake_minimum_required(VERSION 3.16)
project(RaylibBox2DFlecs C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Disable shared libs globally (static by default)
set(BUILD_SHARED_LIBS OFF)

# Skip RPATH to avoid dynamic linking hints
set(CMAKE_SKIP_RPATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# Add libraries
add_subdirectory(lib/box2d)
add_subdirectory(lib/raylib)
add_subdirectory(lib/flecs)

# Your source files
add_executable(RaylibBox2DFlecs
    src/main.c
    src/lib/utils.c
    src/systems/onstart.c
    src/systems/preupdate.c
    src/systems/onupdate.c
)

# Include paths
target_include_directories(RaylibBox2DFlecs PRIVATE
    src
    lib/box2d/include
    lib/raylib/src
    lib/flecs/include
)

# Explicitly link static versions of libraries
target_link_libraries(RaylibBox2DFlecs raylib flecs_static box2d)

# Required libraries for Raylib on Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(RaylibBox2DFlecs
        PRIVATE
        m pthread dl rt X11
    )
endif()
