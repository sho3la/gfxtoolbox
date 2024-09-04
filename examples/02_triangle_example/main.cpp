

#include <iostream>

#include "gfx.h"

// global
auto gfx_backend = std::make_shared<gfx::GFX>();

uint32_t vertex_buffer_id, gpu_mesh_id, gpu_program;

const char* vertexShader = "#version 450 core\n"
						   "layout (location = 0) in vec3 aPos;\n"
						   "void main()\n"
						   "{\n"
						   "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
						   "}\0";

const char* fragmentShader = "#version 450 core\n"
							 "out vec4 FragColor;\n"
							 "void main()\n"
							 "{\n"
							 "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
							 "}\n\0";

void
init()
{
	// clang-format off

	float vertices[] = {
	 -0.5f, -0.5f, 0.0f, // left
	  0.5f, -0.5f, 0.0f, // right
	  0.0f,  0.5f, 0.0f  // top
	};

	// clang-format on

	vertex_buffer_id = gfx_backend->createVertexBuffer(vertices, sizeof(vertices), gfx::BUFFER_USAGE::STATIC);
	gpu_mesh_id = gfx_backend->createGPUMesh(vertex_buffer_id);

	// build and compile our shader program
	gpu_program = gfx_backend->createGPUProgram(vertexShader, fragmentShader);
}

void
input(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void
render()
{
	gfx_backend->setClearColor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f));
	gfx_backend->clearBuffer();

	gfx_backend->bindGPUProgram(gpu_program);

	gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES, gpu_mesh_id, 3);
}

int
main()
{
	// load gfx
	// ---------------------------------------
	gfx_backend->init("gfx triangle", 800, 600);
	gfx_backend->start(init, input, render);

	return 0;
}