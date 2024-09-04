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
}

void
input(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int
main()
{
	// initialize gfx
	// ---------------------------------------
	gfx_backend->init("gfx window", 800, 600);
	gfx_backend->start(init, input, render);

	return 0;
}