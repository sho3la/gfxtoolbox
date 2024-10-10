#include <iostream>

#include "gfx.h"
#include "gfx_fbo.h"

#include <imgui.h>

// global
auto gfx_backend = std::make_shared<gfx::GFX>();

uint32_t vertex_buffer_id, index_buffer_id, gpu_mesh_id, gpu_program, gpu_program2;

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
		layout(location = 0) in vec2 position;

		void main() {
			gl_Position = vec4(position, 0.0, 1.0); // Pass through position
		}
		)";

// Fragment Shader Source
const char* fragmentShaderSource = R"(
		#version 450 core
		out vec4 FragColor;

		uniform float scr_height;

		void main()
		{
			vec3 top = vec3(0.8,0.8,0.8);
			vec3 bottom = vec3(0.3,0.3,0.3);
			float gradient = float(gl_FragCoord.y / scr_height);

			// Smooth the gradient by interpolating
			FragColor = vec4(mix(bottom, top, smoothstep(0.0, 1.0, 1 - gradient)), 1.0); // Color from gray (bottom) to light gray (top)
		})";

void
init()
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
	index_buffer_id = gfx_backend->createIndexBuffer(indices, sizeof(indices), gfx::BUFFER_USAGE::STATIC);

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

void
mouse_scroll(double xoffset, double yoffset)
{
}

void
mouse_move(double xpos, double ypos)
{
}

void
mouse_button(int button, int action, int mods)
{
}

void
resize(int width, int height)
{
	if (width == 0 || height == 0)
		return;

	scrn_width = width;
	scrn_height = height;
	frame_buffer->Resize(width, height);
}

void
render_pass_1()
{
	frame_buffer->Bind();
	gfx_backend->setClearColor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f));
	gfx_backend->clearBuffer();

	ImGui::ShowDemoWindow();

	gfx_backend->bindGPUProgram(gpu_program2);
	gfx_backend->setGPUProgramFloat(gpu_program2, "scr_height", scrn_height);

	gfx_backend->draw_indexed(gfx::GFX_Primitive::TRIANGLES, gpu_mesh_id, 6);

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
	gfx_backend->on_MouseScroll(mouse_scroll);
	gfx_backend->on_MouseMove(mouse_move);
	gfx_backend->on_MouseButton(mouse_button);

	gfx_backend->start();

	return 0;
}