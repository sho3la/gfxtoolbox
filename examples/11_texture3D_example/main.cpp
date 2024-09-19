#include <iostream>

#include "gfx.h"

#include <imgui.h>

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

// global
auto gfx_backend = std::make_shared<gfx::GFX>();

int scrn_width = 800;
int scrn_height = 600;
uint32_t vertex_buffer_id, gpu_mesh_id, gpu_program;

OrbitalCamera camera(68.0f, -0.1f, 42.0f);

// create transformations
glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

struct Vert
{
	glm::vec3 pos;
	glm::vec3 color;
};

std::vector<Vert> verts;

glm::vec3 grid_color{0.9, 0.9, 0.9};

// clang-format off

const char* vertexShader = R"(
		#version 450 core

		layout (location = 0) in vec3 v_pos;
		layout (location = 1) in vec3 v_col;

		uniform mat4 mvp;
		smooth out vec3 f_col;

		void main() {
			gl_Position = mvp * vec4(v_pos, 1.0f);
			f_col = v_col;
		})";

const char* fragmentShader =R"(
		#version 450 core

		layout (location = 0) out vec4 out_col;

		smooth in vec3 f_col;

		void main() {
			out_col = vec4(f_col, 1.0f);
		})";

// clang-format on

void
init()
{
	const int R = 25;
	for (int i = -R; i <= R; i++)
	{
		if (i == 0)
		{
			verts.push_back({glm::vec3{-R, 0, i}, glm::vec3{1, 0, 0}});
			verts.push_back({glm::vec3{R, 0, i}, glm::vec3{1, 0, 0}});

			verts.push_back({glm::vec3{i, 0, -R}, glm::vec3{0, 1, 0}});
			verts.push_back({glm::vec3{i, 0, R}, glm::vec3{0, 1, 0}});
			continue;
		}

		verts.push_back({glm::vec3{i, 0, -R}, grid_color});
		verts.push_back({glm::vec3{i, 0, R}, grid_color});

		verts.push_back({glm::vec3{-R, 0, i}, grid_color});
		verts.push_back({glm::vec3{R, 0, i}, grid_color});
	}

	std::vector<float> gpu_arr;
	for (auto el : verts)
	{
		gpu_arr.push_back(el.pos.x);
		gpu_arr.push_back(el.pos.y);
		gpu_arr.push_back(el.pos.z);

		gpu_arr.push_back(el.color.x);
		gpu_arr.push_back(el.color.y);
		gpu_arr.push_back(el.color.z);
	}

	vertex_buffer_id =
		gfx_backend->createVertexBuffer(gpu_arr.data(), sizeof(gpu_arr[0]) * gpu_arr.size(), gfx::BUFFER_USAGE::STATIC);

	gfx::Attributes attributes;
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "POSITION"));
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "COLOR"));

	gpu_mesh_id = gfx_backend->createGPUMesh(vertex_buffer_id, attributes);

	// build and compile our shader program
	gpu_program = gfx_backend->createGPUProgram(vertexShader, fragmentShader);

	// initialize projection matrix
	projection = glm::perspective(glm::radians(45.0f), (float)scrn_width / (float)scrn_height, 0.01f, 10000.0f);
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
	projection = glm::perspective(glm::radians(45.0f), (float)scrn_width / (float)scrn_height, 0.01f, 10000.0f);
}

void
render()
{
	ImGui::ShowDemoWindow();

	gfx_backend->setClearColor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f));
	gfx_backend->clearBuffer();

	gfx_backend->bindGPUProgram(gpu_program);

	view = camera.getViewMatrix();

	// send to gpu
	auto mvp = projection * view * model;
	gfx_backend->setGPUProgramMat4(gpu_program, "mvp", mvp);

	glLineWidth(1.0f);
	gfx_backend->draw(gfx::GFX_Primitive::LINES, gpu_mesh_id, verts.size());
}

int
main()
{
	// initialize gfx
	// ---------------------------------------
	gfx_backend->init("gfx grid 3d", scrn_width, scrn_height);
	gfx_backend->on_Init(init);
	gfx_backend->on_Render(render);
	gfx_backend->on_Resize(resize);
	gfx_backend->on_MouseScroll(mouse_scroll);
	gfx_backend->on_MouseMove(mouse_move);
	gfx_backend->on_MouseButton(mouse_button);

	gfx_backend->start();

	return 0;
}