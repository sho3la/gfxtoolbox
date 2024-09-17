cmake_minimum_required(VERSION 3.16)

set(MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	set(MASTER_PROJECT ON)
endif ()

set(PROJECT_NAME gfxtoolbox)

project(${PROJECT_NAME} LANGUAGES CXX C)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_RPATH "$ORIGIN")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

option(BUILD_EXAMPLES "Build example applications that showcase." ON)

add_subdirectory(external/glew EXCLUDE_FROM_ALL)
add_subdirectory(external/glfw-3.4)
add_subdirectory(external/stb)
add_subdirectory(external/imgui-1.91.1)

add_subdirectory(src)

if (BUILD_EXAMPLES)
	add_subdirectory(examples)
endif ()
