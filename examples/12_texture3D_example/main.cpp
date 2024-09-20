#include <iostream>

#include "gfx.h"
#include "gfx_fbo.h"

#include <imgui.h>

// global
auto gfx_backend = std::make_shared<gfx::GFX>();

uint32_t vertex_buffer_id, index_buffer_id, gpu_mesh_id, gpu_program;

uint32_t vertex_buffer_id2, index_buffer_id2, gpu_mesh_id2, gpu_program2;

int scrn_width = 800;
int scrn_height = 600;
std::shared_ptr<gfx::Framebuffer> frame_buffer;

const char* vertexShader = R"(
		#version 450 core
		layout (location = 0) in vec3 aPos;
		layout (location = 1) in vec2 aTexCoords;

		out vec2 TexCoords;

		void main()
		{
			TexCoords = aTexCoords;
			gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
		})";

const char* fragmentShader = R"(
		#version 450 core
		out vec4 FragColor;
		in vec2 TexCoords;
		uniform sampler2D screenTexture;
		void main()
		{
			vec3 col = texture(screenTexture, TexCoords).rgb;
			FragColor = vec4(col, 1.0);
		})";

// Vertex Shader Source
const char* vertexShaderSource = R"(
		#version 450 core
		layout(location = 0) in vec3 vVertex;

		// combined modelview projection matrix
		uniform mat4 mvp;

		out vec3 vColor;

		void main()
		{ 	 
			//get clipspace position
			vColor = vVertex;
			gl_Position = mvp*vec4(vVertex.xyz,1);
		})";

// Fragment Shader Source
const char* fragmentShaderSource = R"(
		#version 450 core
		in vec3 vColor;

		out vec4 fragColor;

		void main()
		{
			fragColor = vec4(vColor, 1);
		})";

// create transformations
glm::mat4 projection = glm::mat4(1.0f);

inline static void
_frame_buffer_init()
{
	// clang-format off
	float vertices[] = {
		// positions      // texture coords
		 1.0f,  1.0f, 0.0f,  1.0f, 0.0f, // top right
		 1.0f, -1.0f, 0.0f,  1.0f, 1.0f, // bottom right
		-1.0f, -1.0f, 0.0f,  0.0f, 1.0f, // bottom left
		-1.0f,  1.0f, 0.0f,  0.0f, 0.0f  // top left 
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
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC2, "TEXCOORD"));

	gpu_mesh_id = gfx_backend->createGPUMesh(vertex_buffer_id, index_buffer_id, attributes);

	// build and compile our shader program
	gpu_program = gfx_backend->createGPUProgram(vertexShader, fragmentShader);
	gpu_program2 = gfx_backend->createGPUProgram(vertexShaderSource, fragmentShaderSource);

	// create another render buffer to render on it.
	frame_buffer = std::make_shared<gfx::Framebuffer>(scrn_width, scrn_height);
}

inline static void
_cube_scene_init()
{
	// clang-format off

	float vertices[] = {

		// positions        
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f, 
		0.5f,  0.5f, -0.5f, 
		0.5f,  0.5f, -0.5f, 
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f, 
		0.5f,  0.5f,  0.5f, 
		0.5f,  0.5f,  0.5f, 
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		0.5f,  0.5f,  0.5f, 
		0.5f,  0.5f, -0.5f, 
		0.5f, -0.5f, -0.5f, 
		0.5f, -0.5f, -0.5f, 
		0.5f, -0.5f,  0.5f, 
		0.5f,  0.5f,  0.5f, 

		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f, 
		0.5f, -0.5f,  0.5f, 
		0.5f, -0.5f,  0.5f, 
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f, 
		0.5f,  0.5f,  0.5f, 
		0.5f,  0.5f,  0.5f, 
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};

	// clang-format on

	vertex_buffer_id2 = gfx_backend->createVertexBuffer(vertices, sizeof(vertices), gfx::BUFFER_USAGE::STATIC);

	gfx::Attributes attributes;
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "POSITION"));

	gpu_mesh_id2 = gfx_backend->createGPUMesh(vertex_buffer_id2, attributes);

	// build and compile our shader program
	gpu_program2 = gfx_backend->createGPUProgram(vertexShaderSource, fragmentShaderSource);
}

void
init()
{
	_frame_buffer_init();

	_cube_scene_init();

	// initialize projection matrix
	projection = glm::perspective(glm::radians(45.0f), (float)scrn_width / (float)scrn_height, 0.1f, 100.0f);
}

void
resize(int width, int height)
{
	if (width == 0 || height == 0)
		return;

	scrn_width = width;
	scrn_height = height;
	frame_buffer->Resize(width, height);
	projection = glm::perspective(glm::radians(45.0f), (float)scrn_width / (float)scrn_height, 0.01f, 10000.0f);
}

void
render_pass_1()
{
	frame_buffer->Bind();
	gfx_backend->setClearColor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f));
	gfx_backend->clearBuffer();

	gfx_backend->bindGPUProgram(gpu_program2);
	
	glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::mat4 view = glm::mat4(1.0f);

	model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

	auto mvp = projection * view * model;

	gfx_backend->setGPUProgramMat4(gpu_program2, "mvp", mvp);

	gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES, gpu_mesh_id2, 36);

	frame_buffer->Unbind();
}

void
render()
{
	// render to frame buffer
	render_pass_1();

	// render to main buffer
	gfx_backend->setClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	gfx_backend->clearBuffer();

	gfx_backend->bindTexture2D(frame_buffer->GetTexture());
	gfx_backend->bindGPUProgram(gpu_program);

	gfx_backend->draw_indexed(gfx::GFX_Primitive::TRIANGLES, gpu_mesh_id, 6);
}

int
main()
{
	// initialize gfx
	// ---------------------------------------
	gfx_backend->init("gfx frame buffer", scrn_width, scrn_height);
	gfx_backend->on_Init(init);
	gfx_backend->on_Render(render);
	gfx_backend->on_Resize(resize);

	gfx_backend->start();

	return 0;
}