cmake_minimum_required(VERSION 3.30.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "CMake version: ${CMAKE_PREFIX_PATH}")

project(xnb)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(SOURCES
	src/LzxDecoder.cpp
	src/bufferReader.cpp
	src/reader.cpp
	src/xnb.cpp
)

add_executable(xnb ${SOURCES} main.cpp)

# install
# file(GLOB_RECURSE HEADERS include/*.h)
#
# install(TARGETS xnb EXPORT xnbTargets DESTINATION lib)
# install(FILES ${HEADERS} DESTINATION include/xnb)
#
# install(EXPORT xnbTargets
# 	FILE xnbTargets.cmake
# 	NAMESPACE xnb::
# 	DESTINATION lib/cmake/xnb
# )
#
# include(CMakePackageConfigHelpers)
# configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
# 	"${CMAKE_CURRENT_BINARY_DIR}/xnbConfig.cmake"
#   INSTALL_DESTINATION "lib/cmake/xnb"
#   NO_SET_AND_CHECK_MACRO
#   NO_CHECK_REQUIRED_COMPONENTS_MACRO
# )
# write_basic_package_version_file(
#   "${CMAKE_CURRENT_BINARY_DIR}/xnbConfigVersion.cmake"
#   VERSION 1.0.0
#   COMPATIBILITY AnyNewerVersion
# )
#
# install(FILES
# 	${CMAKE_CURRENT_BINARY_DIR}/xnbConfig.cmake
#   ${CMAKE_CURRENT_BINARY_DIR}/xnbConfigVersion.cmake
#   DESTINATION lib/cmake/xnb
# )
