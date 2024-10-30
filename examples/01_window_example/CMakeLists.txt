cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME 01_window_example)

add_executable(${PROJECT_NAME}
	KHR/khrplatform.h
	main.cpp
	glad/glad.h
	glad/glad.c
 )

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Examples)

target_link_libraries(${PROJECT_NAME}
	glfw
)

if (DEFINED EMSCRIPTEN)
set_target_properties(${PROJECT_NAME}
        PROPERTIES SUFFIX ".html"
        LINK_FLAGS " --bind -s USE_GLFW=3 -s FULL_ES3=1 -s WASM=1")
endif (DEFINED EMSCRIPTEN)