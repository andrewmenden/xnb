cmake_minimum_required(VERSION 3.30.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "CMake version: ${CMAKE_PREFIX_PATH}")

project(xnb)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(SOURCES
	src/xnb.cpp
)

add_library(xnb ${SOURCES})

# installing

install(TARGETS xnb EXPORT xnbConfig
		ARCHIVE DESTINATION lib
		INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)
