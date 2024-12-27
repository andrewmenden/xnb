cmake_minimum_required(VERSION 3.30.0)

project(xnb)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# this was built with mingw (32-bit), feel free to comment this out
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

# debug build
# set(CMAKE_BUILD_TYPE Debug)

include_directories(include)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(xnb ${SOURCES} main.cpp)
