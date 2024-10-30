cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME 08_orbit_camera_example)

add_executable(${PROJECT_NAME} main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Examples)

target_link_libraries(${PROJECT_NAME}
	gfx
)

target_compile_definitions(${PROJECT_NAME}
	PRIVATE
	DATA_DIR="${CMAKE_SOURCE_DIR}/GL4/examples/data/"
)