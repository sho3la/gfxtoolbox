cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME gfx)

# list the header files
set(HEADER_FILES
	gfx.h
	enums.h
	attributes.h
	gpu_attribute.h
	Image.h
)

set(SOURCE_FILES
	gfx.cpp
	attributes.cpp
	gpu_attribute.cpp
	Image.cpp
)

# add library target
add_library(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

# list linked libraries
target_link_libraries(${PROJECT_NAME}
	glfw
	glew
	stb
)

# list include directories
target_include_directories(${PROJECT_NAME}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/
	${CMAKE_SOURCE_DIR}/external/glew/include
	${CMAKE_SOURCE_DIR}/external/glfw-3.4/include
	${CMAKE_SOURCE_DIR}/external/glm-1.0.1/
)

# enable C++17
# disable any compiler specifc extensions
# add d suffix in debug mode
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_EXTENSIONS OFF
	DEBUG_POSTFIX d
)

# generate exports header file
include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME}
	EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/Exports.h
)