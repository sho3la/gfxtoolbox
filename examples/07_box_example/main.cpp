

#include <iostream>

#include "gfx.h"

// global
auto gfx_backend = std::make_shared<gfx::GFX>();
uint32_t vertex_buffer_id, gpu_mesh_id, gpu_program, texture2d_id;

// clang-format off

const char* vertexShader =
		"#version 450 core													\n	\
		layout(location = 0) in vec3 aPos;									\n	\
		layout(location = 1) in vec2 aTexCoord;								\n	\
																			\n	\
		out vec2 TexCoord;													\n	\
																			\n	\
		uniform mat4 model;													\n	\
		uniform mat4 view;													\n	\
		uniform mat4 projection;											\n	\
																			\n	\
		void main()															\n	\
		{																	\n	\
			gl_Position = projection * view * model * vec4(aPos, 1.0f);		\n	\
			TexCoord = vec2(aTexCoord.x, aTexCoord.y);						\n	\
		} \n";

const char* fragmentShader =
		"#version 450 core								\n	\
		out vec4 FragColor;								\n	\
		in vec2 TexCoord;								\n	\
														\n	\
		// texture sampler								\n	\
		uniform sampler2D texture1;						\n	\
														\n	\
		void main()										\n	\
		{												\n	\
			FragColor = texture(texture1, TexCoord);	\n	\
		} \n";

// clang-format on

void
init()
{
	// clang-format off

	float vertices[] = {

		// positions          // texcoord
		-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,    1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,    1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,   1.0f, 0.0f,

		0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,    0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,    1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,    1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,    1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,   0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,   0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,   0.0f, 1.0f
	};

	// clang-format on

	vertex_buffer_id = gfx_backend->createVertexBuffer(vertices, sizeof(vertices), gfx::BUFFER_USAGE::STATIC);

	gfx::Attributes attributes;
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "POSITION"));
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC2, "TEXCOORD"));

	gpu_mesh_id = gfx_backend->createGPUMesh(vertex_buffer_id, attributes);

	// build and compile our shader program
	gpu_program = gfx_backend->createGPUProgram(vertexShader, fragmentShader);

	// load and create a texture
	// -------------------------
	auto image = std::make_shared<gfx::Image>(DATA_DIR "opengl.jpg");

	texture2d_id = gfx_backend->createTexture2D(
		image.get(),
		gfx::Wrapping_Mode::REPEAT,
		gfx::Filtering_Mode::LINEAR,
		gfx::Filtering_Mode::LINEAR,
		false);
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

	gfx_backend->bindTexture2D(texture2d_id);
	gfx_backend->bindGPUProgram(gpu_program);

	// create transformations
	glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);

	gfx_backend->setGPUProgramMat4(gpu_program, "model", model);
	gfx_backend->setGPUProgramMat4(gpu_program, "view", view);
	gfx_backend->setGPUProgramMat4(gpu_program, "projection", projection);

	gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES, gpu_mesh_id, 36);
}

int
main()
{
	// initialize gfx
	// ---------------------------------------
	gfx_backend->init("gfx box 3d", 800, 600);
	gfx_backend->start(init, input, render);

	return 0;
}