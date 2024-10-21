#include "gfx.h"
#include "gfx_fbo.h"

#include <imgui.h>
#include <iostream>

// global
struct OrbitalCamera
{
	OrbitalCamera(float distance, float azimuth, float elevation)
		: distance(distance), azimuth(azimuth), elevation(elevation), target(glm::vec3(0.0f, 0.0f, 0.0f))
	{
	}

	glm::mat4
	getViewMatrix()
	{
		// Convert spherical coordinates to Cartesian coordinates
		float x = distance * cos(glm::radians(elevation)) * cos(glm::radians(azimuth));
		float y = distance * sin(glm::radians(elevation));
		float z = distance * cos(glm::radians(elevation)) * sin(glm::radians(azimuth));

		// Create the camera position
		glm::vec3 cameraPosition(x, y, z);

		// The view matrix is the inverse of the look-at matrix
		return glm::lookAt(cameraPosition, target, glm::vec3(0, 1, 0));
	}

	void
	zoom(float yoffset)
	{
		distance = distance - (float)yoffset * zoomSpeed;
	}

	void
	rotate(float xpos, float ypos)
	{
		float xoffset = xpos - start_pos.x;
		float yoffset = ypos - start_pos.y;
		start_pos = glm::vec2(xpos, ypos);

		xoffset *= rot_sensitivity;
		yoffset *= rot_sensitivity;

		azimuth = azimuth + xoffset;
		elevation = elevation + yoffset;

		// clamp to range [-90, 90] around the horizontal axis
		elevation = glm::clamp(elevation, -90.0f, 90.0f);
	}

	float distance;	 // Distance from the target
	float azimuth;	 // Rotation around the vertical axis (in degrees)
	float elevation; // Rotation around the horizontal axis (in degrees)

	glm::vec3 target; // point to look at

	float zoomSpeed = 5.0f;
	float rot_sensitivity = 0.5f;

	bool is_draging = false;
	glm::vec2 start_pos;
};

OrbitalCamera camera(68.0f, -0.1f, 42.0f);

auto gfx_backend = std::make_shared<gfx::GFX>();

int scrn_width = 800;
int scrn_height = 600;

uint32_t vertex_buffer_id, gpu_mesh_id, gpu_program;

// create transformations
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

// clang-format off

const char* vertexShader = R"(
		#version 450 core

		layout (location = 0) in vec3 Position;

		out vec2 v;

		void main()
		{
			v = Position.xy;
			gl_Position = vec4(Position, 1.0);
		})";

const char* fragmentShader =R"(
		#version 450 core
		uniform mat4 inv_view;
		uniform vec2 resolution;
		uniform vec3 lightPos;

		in vec2 v;

		out vec4 FragColor;

		vec3 skyColor( in vec3 rd )
		{
			vec3 sundir = normalize( lightPos );

			float yd = min(rd.y, 0.);
			rd.y = max(rd.y, 0.);

			vec3 col = vec3(0.);

			col += vec3(0.9, 0.9 , 0.9) * exp(-rd.y * 9.0); // Red / Green
			col += vec3(0.3, 0.5, 0.7) * (1. - exp(-rd.y * 8.0) ) * exp(-rd.y * 0.9); // Blue

			col = mix(col*1.2, vec3(0.8),  1.-exp(yd*100.)); // Fog

			col += vec3(1.0, 0.8, 0.55) * pow( max(dot(rd,sundir),0.), 15. ) * .3; // Sun
			col += vec3(1.0, 0.5, 0.3) * pow(max(dot(rd, sundir),0.), 300.0) ;

			return col;
		}

		void main()
		{
			vec3 dir = vec3( normalize( inv_view * vec4(v.xy*vec2(resolution.x/resolution.y,1),-1.5,0.0) ) );
			FragColor = vec4(skyColor(dir),1.0);
		})";

// clang-format on

void
init()
{
	// clang-format off
	float vertices[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f 
	};
	// clang-format on

	vertex_buffer_id = gfx_backend->createVertexBuffer(vertices, sizeof(vertices), gfx::BUFFER_USAGE::STATIC);

	gfx::Attributes attributes;
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "POSITION"));

	gpu_mesh_id = gfx_backend->createGPUMesh(vertex_buffer_id, attributes);

	// build and compile our shader program
	gpu_program = gfx_backend->createGPUProgram(vertexShader, fragmentShader);

	projection = glm::perspective(glm::radians(45.0f), (float)scrn_width / (float)scrn_height, 0.1f, 1000.0f);
}

void
render()
{
	gfx_backend->setClearColor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f));
	gfx_backend->clearBuffer();

	gfx_backend->bindGPUProgram(gpu_program);
	
	view = camera.getViewMatrix();
	gfx_backend->setGPUProgramMat4(gpu_program, "inv_view", glm::inverse(view));
	gfx_backend->setGPUProgramVec2(gpu_program, "resolution", glm::vec2(scrn_width, scrn_height));
	gfx_backend->setGPUProgramVec3(gpu_program, "lightPos", glm::vec3(50));

	glDisable(GL_DEPTH_TEST);
	gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES_STRIP, gpu_mesh_id, 4);
	glEnable(GL_DEPTH_TEST);
}

void
mouse_scroll(double xoffset, double yoffset)
{
	camera.zoom(yoffset);
}

void
mouse_move(double xpos, double ypos)
{
	if (camera.is_draging)
	{
		camera.rotate(xpos, ypos);
	}
}

void
mouse_button(int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && camera.is_draging == false)
	{
		camera.is_draging = true;
		camera.start_pos = gfx_backend->getMouse_position();
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		camera.is_draging = false;
	}
}

void
resize(int width, int height)
{
	if (width == 0 || height == 0)
		return;

	scrn_width = width;
	scrn_height = height;
	projection = glm::perspective(glm::radians(45.0f), (float)scrn_width / (float)scrn_height, 0.1f, 1000.0f);
}

int
main()
{
	// initialize gfx
	// ---------------------------------------
	gfx_backend->init("shadow mapping example", scrn_width, scrn_height);
	gfx_backend->on_Init(init);
	gfx_backend->on_Render(render);
	gfx_backend->on_Resize(resize);
	gfx_backend->on_MouseScroll(mouse_scroll);
	gfx_backend->on_MouseMove(mouse_move);
	gfx_backend->on_MouseButton(mouse_button);
	gfx_backend->start();

	return 0;
}