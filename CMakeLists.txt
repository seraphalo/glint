cmake_minimum_required(VERSION 3.16)
project(glint CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(glint
    src/main.cpp
    src/rasterizer/framebuffer.cpp
    src/rasterizer/triangle.cpp
    src/rasterizer/vec.cpp
    src/rasterizer/mat4.cpp
    src/rasterizer/pipeline.cpp
)

# Homebrew on Apple Silicon
target_include_directories(glint PRIVATE
    src
    /opt/homebrew/include
)
target_link_directories(glint PRIVATE /opt/homebrew/lib)
target_link_libraries(glint PRIVATE SDL2)

target_compile_options(glint PRIVATE
    -Wall -Wextra -Wpedantic
)
