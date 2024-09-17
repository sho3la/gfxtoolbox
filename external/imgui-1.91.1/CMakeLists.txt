cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME imgui)

# list the header files
set(HEADER_FILES
	imgui.h
	imconfig.h
	imgui_internal.h
	imstb_rectpack.h
	imstb_textedit.h
	imstb_truetype.h
	imfilebrowser.h
	backends/imgui_impl_opengl3.h
	backends/imgui_impl_glfw.h
)

# list the source files
set(SOURCE_FILES
	imgui.cpp
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_widgets.cpp
	imgui_tables.cpp
	backends/imgui_impl_opengl3.cpp
	backends/imgui_impl_glfw.cpp
)

# add library target
add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
	PRIVATE ${HEADER_FILES}
			${SOURCE_FILES}
)

if (SYMBYO_UNITY_BUILD)
	set_target_properties(${PROJECT_NAME}
		PROPERTIES UNITY_BUILD_BATCH_SIZE 0
				   UNITY_BUILD true)
endif()


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

target_link_libraries(${PROJECT_NAME}
	glfw
)

# list include directories
target_include_directories(${PROJECT_NAME}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/external/glfw-3.4/include
)
