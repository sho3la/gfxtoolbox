

#include <iostream>

#include "gfx.h"

// global
auto gfx_backend = std::make_shared<gfx::GFX>();

uint32_t vertex_buffer_id, index_buffer_id, gpu_mesh_id, gpu_program;

// clang-format off

const char* vertexShader =
		"#version 450 core								\n	\
		layout(location = 0) in vec3 aPos;				\n	\
		layout(location = 1) in vec3 aColor;			\n	\
														\n	\
		out vec3 ourColor;								\n	\
														\n	\
		void main()										\n	\
		{												\n	\
			gl_Position = vec4(aPos, 1.0);				\n	\
			ourColor = aColor;							\n	\
		} \n";

const char* fragmentShader =
		"#version 450 core								\n	\
		out vec4 FragColor;								\n	\
		in vec3 ourColor;								\n	\
														\n	\
														\n	\
		void main()										\n	\
		{												\n	\
			FragColor = vec4(ourColor, 1.0f);			\n	\
		} \n";

// clang-format on

void
init()
{
	// clang-format off

	float vertices[] = {
		// positions          // colors
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f, // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f, // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f  // top left 
	};

	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	// clang-format on

	vertex_buffer_id = gfx_backend->createVertexBuffer(vertices, sizeof(vertices), gfx::BUFFER_USAGE::STATIC);
	index_buffer_id = gfx_backend->createVertexBuffer(indices, sizeof(indices), gfx::BUFFER_USAGE::STATIC);

	gfx::Attributes attributes;
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "POSITION"));
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "COLOR"));

	gpu_mesh_id = gfx_backend->createGPUMesh(vertex_buffer_id, index_buffer_id, attributes);

	// build and compile our shader program
	gpu_program = gfx_backend->createGPUProgram(vertexShader, fragmentShader);
}

void
render()
{
	gfx_backend->setClearColor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f));
	gfx_backend->clearBuffer();

	gfx_backend->bindGPUProgram(gpu_program);

	gfx_backend->draw_indexed(gfx::GFX_Primitive::TRIANGLES, gpu_mesh_id, 6);
}

int
main()
{
	// initialize gfx
	// ---------------------------------------
	gfx_backend->init("gfx colors", 800, 600);
	gfx_backend->on_Init(init);
	gfx_backend->on_Render(render);
	gfx_backend->start();

	return 0;
}