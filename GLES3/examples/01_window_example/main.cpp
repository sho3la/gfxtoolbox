#include "gfx.h"

// global
auto gfx_backend = std::make_shared<gfx::GFX>();

void
init()
{
}

void
render()
{
	gfx_backend->setClearColor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f));
	gfx_backend->clearBuffer();

	printf("outside \n");
}

int
main()
{
	// initialize gfx
	// ---------------------------------------
	gfx_backend->init("gfx window", 800, 600);
	gfx_backend->on_Init(init);
	gfx_backend->on_Render(render);
	gfx_backend->start();

	return 0;
}


//#include <stdio.h>
//#include <stdlib.h>
//
//#include "glad/glad.h"
//#include <GLFW/glfw3.h>
//
//#ifdef __EMSCRIPTEN__
//#include <emscripten/emscripten.h>
//#endif
//
//const GLuint WIDTH = 800, HEIGHT = 600;
//
//void
//key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
//{
//	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, GL_TRUE);
//}
//
//void
//render_frame(GLFWwindow* window)
//{
//	glfwPollEvents();
//
//	glClearColor(0.7f, 0.9f, 0.1f, 1.0f);
//	glClear(GL_COLOR_BUFFER_BIT);
//
//	glfwSwapBuffers(window);
//}
//
//int
//main(void)
//{
//	glfwInit();
//
//	// Set GLFW to use OpenGL ES 3.2
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
//
//	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "[glad] GLES2 with GLFW", NULL, NULL);
//	glfwMakeContextCurrent(window);
//
//	glfwSetKeyCallback(window, key_callback);
//
//	/* Load GLES */
//	int gles_version = gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress);
//
//	printf("GLES %d.%d\n", GLVersion.major, GLVersion.minor);
//
//#ifdef __EMSCRIPTEN__
//	emscripten_set_main_loop_arg((em_arg_callback_func)render_frame, window, 60, 1);
//#else
//	while (!glfwWindowShouldClose(window))
//	{
//		render_frame(window);
//	}
//#endif
//
//	return 0;
//}