cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME glad)

add_library(${PROJECT_NAME} STATIC
	KHR/khrplatform.h
	glad/glad.h
	glad/glad.c
)

# add d suffix in debug mode
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX d)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER External)

# define debug macro
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

# list include directories
target_include_directories(${PROJECT_NAME}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

