cmake_minimum_required(VERSION 3.9)

# list the header files
set(HEADER_FILES
	stb_image.h
)

# list the source files
set(SOURCE_FILES
	stb_image.cpp
)


# add library target
add_library(stb STATIC
	${HEADER_FILES}
	${SOURCE_FILES}
)

# add d suffix in debug mode
set_target_properties(stb PROPERTIES DEBUG_POSTFIX d)

# define debug macro
target_compile_definitions(stb PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

# list include directories
target_include_directories(stb
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/
)
