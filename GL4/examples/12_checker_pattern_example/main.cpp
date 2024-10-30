

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
		elevation = glm::clamp(elevation, -89.0f, 89.0f);
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
uint32_t vertex_buffer_id, gpu_mesh_id, gpu_program, index_buffer_id;

float scale_val = 20.0f;
glm::vec3 color1(1.0f, 1.0f, 1.0f);
glm::vec3 color2(0.8f, 0.8f, 0.8f);

OrbitalCamera camera(68.0f, -0.1f, 42.0f);

// create transformations
glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

// clang-format off

const char* vertexShader =
		R"(
		#version 450 core
		layout(location = 0) in vec3 aPos;
		layout(location = 1) in vec2 aTexCoord;

		out vec2 TexCoord; // Output texture coordinates
		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main()
		{
			gl_Position = projection * view * model * vec4(aPos, 1.0f);
			TexCoord = aTexCoord;
		})";

const char* fragmentShader =
		R"(
		#version 450 core
		out vec4 FragColor;
		in vec2 TexCoord;

		
		uniform float scale; // Adjust this value for larger/smaller squares
		uniform vec3 color1; // white
		uniform vec3 color2; // black

		void main()
		{
			// Scale the checker pattern
			vec2 pos = TexCoord * scale;

			// Create a checker pattern
			float checker = step(0.5, mod(floor(pos.x) + floor(pos.y), 2.0));

			// Calculate the distance to the edges
			float distToEdgeX = abs(fract(pos.x));
			float distToEdgeY = abs(fract(pos.y));
			float edgeDistance = min(distToEdgeX, distToEdgeY);

			// Smooth the transition with a simple step function
			float smoothing = smoothstep(0.0, 0.01, edgeDistance); // Adjust the second parameter for more or less smoothing

			// Mix colors based on the checker pattern and smoothing
			FragColor = vec4(mix(color1, color2, mix(checker, 1.0 - checker, smoothing)), 1.0);
		})";

	float vertices[] = {
	// positions                       // texture coords
	11.045807, 0.001461, -8.080369,     0.62773, 0.515268,
	-11.045931, 0.001461, -8.08037,	    0.000117, 0.515268,
	-11.044489, 0, 10.054064,		    0.000158, 7.9e-05,
	11.047249, 0, 10.054064,		    0.627771, 7.9e-05,
	11.045708, 0.00885, -8.275193,	    0.627728, 0.520807,
	-11.046028, 0.00885, -8.275193,	    0.000114, 0.520807,
	11.04561, 0.030778, -8.46893,	    0.627725, 0.526346,
	-11.046127, 0.030778, -8.46893,	    0.000111, 0.526346,
	11.045513, 0.067122, -8.660492,	    0.627722, 0.531885,
	-11.046225, 0.067122, -8.660492,    0.000108, 0.531885,
	11.045417, 0.117677, -8.848809,	    0.627719, 0.537425,
	-11.046318, 0.117677, -8.848811,    0.000106, 0.537425,
	11.045324, 0.182164, -9.032831,	    0.627717, 0.542965,
	-11.046412, 0.182164, -9.032831,    0.000103, 0.542965,
	11.045234, 0.260218, -9.211525,	    0.627714, 0.548504,
	-11.046502, 0.260218, -9.211525,    0.0001, 0.548504,
	11.045148, 0.351406, -9.383892,	    0.627712, 0.554044,
	-11.046589, 0.351406, -9.383893,    9.8e-05, 0.554044,
	11.045064, 0.455216, -9.548971,	    0.627709, 0.559584,
	-11.046674, 0.455216, -9.548971,    9.6e-05, 0.559584,
	11.044985, 0.571068, -9.705836,	    0.627707, 0.565124,
	-11.046753, 0.571068, -9.705836,    9.3e-05, 0.565124,
	11.044909, 0.698315, -9.853611,	    0.627705, 0.570664,
	-11.046826, 0.698315, -9.853611,    9.1e-05, 0.570664,
	11.04484, 0.836244, -9.99147,	    0.627703, 0.576205,
	-11.046896, 0.836244, -9.99147,	    8.9e-05, 0.576205,
	11.044775, 0.984084, -10.118642,    0.627701, 0.581745,
	-11.046961, 0.984084, -10.118642,   8.7e-05, 0.581745,
	11.044716, 1.141009, -10.234413,    0.627699, 0.587285,
	-11.04702, 1.141009, -10.234413,    8.6e-05, 0.587285,
	11.044663, 1.306139, -10.338138,    0.627698, 0.592825,
	-11.047071, 1.306139, -10.338138,   8.4e-05, 0.592825,
	11.044619, 1.478554, -10.429237,    0.627697, 0.598365,
	-11.047117, 1.478554, -10.429237,   8.3e-05, 0.598365,
	11.04458, 1.657288, -10.5072,	    0.627696, 0.603904,
	-11.047157, 1.657288, -10.5072,	    8.2e-05, 0.603905,
	11.044546, 1.841341, -10.571593,    0.627695, 0.609444,
	-11.04719, 1.841341, -10.571592,    8.1e-05, 0.609444,
	11.044521, 2.029686, -10.622053,    0.627694, 0.614984,
	-11.047215, 2.029686, -10.622053,   8e-05, 0.614984,
	11.044501, 2.221267, -10.658298,    0.627693, 0.620523,
	-11.047235, 2.221267, -10.658298,   8e-05, 0.620523,
	11.044491, 2.415013, -10.680127,    0.627693, 0.626062,
	-11.047244, 2.415013, -10.680127,   7.9e-05, 0.626062,
	11.044487, 2.609842, -10.687416,    0.627693, 0.631601,
	-11.047248, 2.609842, -10.687415,   7.9e-05, 0.631601,
	11.044487, 15.574548, -10.687418,   0.627693, 0.999921,
	-11.047248, 15.574548, -10.687418,  7.9e-05, 0.999921,
	};

	unsigned int indices[] = {
		0, 1, 2, 0, 2,
		3, 1, 0, 4, 1,
		4, 5, 5, 4, 6,
		5, 6, 7, 7, 6,
		8, 7, 8, 9, 9,
		8, 10, 9, 10, 11,
		11, 10, 12, 11, 12,
		13, 13, 12, 14, 13,
		14, 15, 15, 14, 16,
		15, 16, 17, 17, 16,
		18, 17, 18, 19, 19,
		18, 20, 19, 20, 21,
		21, 20, 22, 21, 22,
		23, 23, 22, 24, 23,
		24, 25, 25, 24, 26,
		25, 26, 27, 27, 26,
		28, 27, 28, 29, 29,
		28, 30, 29, 30, 31,
		31, 30, 32, 31, 32,
		33, 33, 32, 34, 33,
		34, 35, 35, 34, 36,
		35, 36, 37, 37, 36,
		38, 37, 38, 39, 39,
		38, 40, 39, 40, 41,
		41, 40, 42, 41, 42,
		43, 43, 42, 44, 43,
		44, 45, 45, 44, 46,
		45, 46, 47
	};

// clang-format on

void
init()
{
	vertex_buffer_id = gfx_backend->createVertexBuffer(vertices, sizeof(vertices), gfx::BUFFER_USAGE::STATIC);
	index_buffer_id = gfx_backend->createIndexBuffer(indices, sizeof(indices), gfx::BUFFER_USAGE::STATIC);

	gfx::Attributes attributes;
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "POSITION"));
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC2, "TEXCOORD"));

	gpu_mesh_id = gfx_backend->createGPUMesh(vertex_buffer_id, index_buffer_id, attributes);

	// build and compile our shader program
	gpu_program = gfx_backend->createGPUProgram(vertexShader, fragmentShader);

	// initialize projection matrix
	projection = glm::perspective(glm::radians(45.0f), (float)scrn_width / (float)scrn_height, 0.1f, 10000.0f);
}

void
render()
{
	gfx_backend->setClearColor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f));
	gfx_backend->clearBuffer();

	gfx_backend->bindGPUProgram(gpu_program);

	view = camera.getViewMatrix();

	gfx_backend->setGPUProgramMat4(gpu_program, "model", model);
	gfx_backend->setGPUProgramMat4(gpu_program, "view", view);
	gfx_backend->setGPUProgramMat4(gpu_program, "projection", projection);

	ImGui::ColorEdit3("Color1", glm::value_ptr(color1)); // RGB color picker
	ImGui::ColorEdit3("Color2", glm::value_ptr(color2)); // RGB color picker

	// Create a slider for a float value from 1.0 to 50.0
	ImGui::SliderFloat("checker scale", &scale_val, 1.0f, 50.0f);

	gfx_backend->setGPUProgramFloat(gpu_program, "scale", scale_val);
	gfx_backend->setGPUProgramVec3(gpu_program, "color1", color1);
	gfx_backend->setGPUProgramVec3(gpu_program, "color2", color2);

	gfx_backend->draw_indexed(gfx::GFX_Primitive::TRIANGLES, gpu_mesh_id, sizeof(indices));
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
	scrn_width = width;
	scrn_height = height;
	projection = glm::perspective(glm::radians(45.0f), (float)scrn_width / (float)scrn_height, 0.01f, 10000.0f);
}

int
main()
{
	// initialize gfx
	// ---------------------------------------
	gfx_backend->init("checker pattern 3d", scrn_width, scrn_height);
	gfx_backend->on_Init(init);
	gfx_backend->on_Render(render);
	gfx_backend->on_Resize(resize);
	gfx_backend->on_MouseScroll(mouse_scroll);
	gfx_backend->on_MouseMove(mouse_move);
	gfx_backend->on_MouseButton(mouse_button);
	gfx_backend->start();

	return 0;
}