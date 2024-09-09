

#include <iostream>

#include "gfx.h"

struct CameraState
{
	// angles.x is the rotation of the camera around the global vertical axis, affected by mouse.x
	// angles.y is the rotation of the camera around its local horizontal axis, affected by mouse.y
	glm::vec2 angles = {0.8f, 0.5f};
	// zoom is the position of the camera along its local forward axis, affected by the scroll wheel
	float zoom = -1.2f;
};

struct OrbitCamera
{
	// Whether a drag action is ongoing (i.e., we are between mouse press and mouse release)
	bool is_draging = false;
	// The position of the mouse at the beginning of the drag action
	glm::vec2 startMouse;
	// The camera state at the beginning of the drag action
	CameraState startCameraState;

	// Constant settings
	float sensitivity = 0.005f;
	float scrollSensitivity = 0.1f;
};

// global
auto gfx_backend = std::make_shared<gfx::GFX>();

int scrn_width = 800;
int scrn_height = 600;
uint32_t vertex_buffer_id, gpu_mesh_id, gpu_program, texture2d_id;

CameraState camState;
OrbitCamera orbitcam;

// create transformations
glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

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

	// initialize projection matrix
	projection = glm::perspective(glm::radians(45.0f), (float)scrn_width / (float)scrn_height, 0.01f, 100.0f);
}

void
scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camState.zoom += orbitcam.scrollSensitivity * static_cast<float>(yoffset);
	camState.zoom = glm::clamp(camState.zoom, -2.0f, 2.0f);
}

void
input(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	glfwSetScrollCallback(window, scroll_callback);

	auto lbutton_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (lbutton_state == GLFW_PRESS && !orbitcam.is_draging)
	{
		orbitcam.is_draging = true;
		orbitcam.startMouse = glm::vec2(-(float)xpos, (float)ypos);
		orbitcam.startCameraState = camState;
	}
	else if (lbutton_state == GLFW_RELEASE)
	{
		orbitcam.is_draging = false;
	}

	if (orbitcam.is_draging)
	{
		glm::vec2 currentMouse = glm::vec2(-(float)xpos, (float)ypos);
		glm::vec2 delta = (currentMouse - orbitcam.startMouse) * orbitcam.sensitivity;
		camState.angles = orbitcam.startCameraState.angles + delta;

		// Clamp to avoid going too far when orbitting up/down
		camState.angles.y = glm::clamp(camState.angles.y, -glm::pi<float>() / 2 + 1e-5f, glm::pi<float>() / 2 - 1e-5f);
	}
}

void
render()
{
	gfx_backend->setClearColor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f));
	gfx_backend->clearBuffer();

	gfx_backend->bindTexture2D(texture2d_id);
	gfx_backend->bindGPUProgram(gpu_program);

	// update view matrix
	float cx = cos(camState.angles.x);
	float sx = sin(camState.angles.x);
	float cy = cos(camState.angles.y);
	float sy = sin(camState.angles.y);
	glm::vec3 position = glm::vec3(cx * cy, sx * cy, sy) * std::exp(-camState.zoom);
	view = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0, 0, 1));

	// send to gpu
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
	gfx_backend->init("gfx Orbit Camera", scrn_width, scrn_height);
	gfx_backend->start(init, input, render);

	return 0;
}