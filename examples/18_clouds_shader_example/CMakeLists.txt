cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME 18_clouds_shader_example)

add_executable(${PROJECT_NAME} main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Examples)

target_link_libraries(${PROJECT_NAME}
	gfx
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/
	${CMAKE_SOURCE_DIR}/external/glew/include
	${CMAKE_SOURCE_DIR}/external/glfw-3.4/include
)

target_compile_definitions(${PROJECT_NAME}
	PRIVATE
	DATA_DIR="${CMAKE_SOURCE_DIR}/examples/data/"
)