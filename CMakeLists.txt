cmake_minimum_required(VERSION 3.16)
project(DaGame C)

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

file(GLOB LIB_SOURCES "src/lib/*.c")
file(GLOB ENTITY_SOURCES "src/entities/*.c")
file(GLOB SYSTEM_SOURCES "src/systems/*.c")

# Your source files
add_executable(DaGame
    src/main.c
    ${LIB_SOURCES}
    ${ENTITY_SOURCES}
    ${SYSTEM_SOURCES}
)

# Include paths
target_include_directories(DaGame PRIVATE
    src
    lib/box2d/include
    lib/raylib/src
    lib/raygui/src
    lib/flecs/include
)

# Explicitly link static versions of libraries
target_link_libraries(DaGame raylib flecs_static box2d)

# Required libraries for Raylib on Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(DaGame
        PRIVATE
        m pthread dl rt X11
    )
endif()
