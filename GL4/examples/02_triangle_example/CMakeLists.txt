cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME 02_triangle_example)

add_executable(${PROJECT_NAME} main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Examples)

target_link_libraries(${PROJECT_NAME}
	gfx
)