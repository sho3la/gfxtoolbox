#include <iostream>

#include "gfx.h"
#include "gfx_fbo.h"

#include <imgui.h>

#include <iostream>
#include <fstream>

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

	glm::vec3
	getForwardVector()
	{
		float x = cos(glm::radians(elevation)) * cos(glm::radians(azimuth));
		float y = sin(glm::radians(elevation));
		float z = cos(glm::radians(elevation)) * sin(glm::radians(azimuth));

		// Create the camera position
		glm::vec3 cameraPosition(x, y, z);

		// Normalize the forward vector
		glm::vec3 forward = glm::normalize(cameraPosition - target);

		return forward;
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

	float zoomSpeed = 1.0f;
	float rot_sensitivity = 0.5f;

	bool is_draging = false;
	glm::vec2 start_pos;
};

OrbitalCamera camera(10.0f, -0.1f, 42.0f);

// global
auto gfx_backend = std::make_shared<gfx::GFX>();

uint32_t vertex_buffer_id, index_buffer_id, gpu_mesh_id, gpu_program;

uint32_t vertex_buffer_id2, index_buffer_id2, gpu_mesh_id2, gpu_program2;

int scrn_width = 800;
int scrn_height = 600;
std::shared_ptr<gfx::Framebuffer> frame_buffer;

float mind = -1000, maxd = 3094;
float scalar = 0.005f;

// create transformations
glm::mat4 projection = glm::mat4(1.0f);

unsigned int tex3d;

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
			#version 450

			in vec3 input_position;
			out vec3 entryPoint;

			// Uniforms
			uniform  mat4 model;
			uniform  mat4 view;
			uniform  mat4 mvp;

			void main()
			{
				vec4 cs_position = mvp * vec4(input_position, 1.0);
				vec3 uv = input_position + vec3(0.5,0.5,0.5);

				gl_Position = cs_position;
				entryPoint = uv; // Pass the entry point
			})";

// Fragment Shader Source
const char* fragmentShaderSource = R"(
		#version 450

		in vec3 entryPoint;
		out vec4 fragColor;

		uniform sampler3D volume; // 3D texture containing DICOM intensity data
		uniform float minIntensity; // Minimum intensity value
		uniform float maxIntensity; // Maximum intensity value

		uniform vec3 ray_step; // Maximum intensity value

		void main() {

			vec3 voxelCoord = entryPoint; // Start at the entry point
			vec4 colorAccum = vec4(0.0); // Accumulated color

			// Sample along the ray for a fixed number of iterations
			for (int i = 0; i < 200; i++)
			 {
				// Sample the red channel from the 3D texture
				float intensity = texture(volume, voxelCoord).r;

				// Normalize the intensity
				float normalizedIntensity = (intensity - minIntensity) / (maxIntensity - minIntensity);
				normalizedIntensity = clamp(normalizedIntensity, 0.0, 1.0); // Clamp to [0, 1]

				float prev_alpha = normalizedIntensity * (1.0 - colorAccum.a);

				// Accumulate color as grayscale
				colorAccum += vec4(normalizedIntensity, normalizedIntensity, normalizedIntensity, prev_alpha);
				colorAccum.a += prev_alpha;

				// Move to the next voxel along the ray
				voxelCoord += ray_step;

				 if (colorAccum.a > 0.99)
					break;

			}

			// Finalize the color output
			fragColor = colorAccum;

		}


)";

inline static void
_init_framebuf()
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

	// create another render buffer to render on it.
	frame_buffer = std::make_shared<gfx::Framebuffer>(scrn_width, scrn_height);
}


// init 3D texture to store the volume data used fo ray casting
GLuint
initVol3DTex()
{
	unsigned int g_volTexObj;
	auto w = 0;
	auto h = 0;
	auto d = 0;

	///////////////////////
	std::ifstream infile(DATA_DIR "img_3d.txt");
	if (!infile)
	{
		std::cerr << "Error opening file for reading: " << DATA_DIR "img_3d.txt" << std::endl;
	}

	std::string line;
	std::getline(infile, line);

	// split first line to 3 dimension by space.
	{
		std::vector<std::string> words;
		std::string::size_type start = 0;
		std::string::size_type end = 0;

		while ((end = line.find(' ', start)) != std::string::npos)
		{
			words.push_back(line.substr(start, end - start));
			start = end + 1;
		}
		words.push_back(line.substr(start)); // Add the last word

		w = std::stof(words[0]);
		h = std::stof(words[1]);
		d = std::stof(words[2]);
	}

	std::vector<float> data;
	while (std::getline(infile, line))
	{
		auto val = std::stof(line);
		data.push_back(val);
	}

	infile.close();

	glGenTextures(1, &g_volTexObj);

	// bind 3D texture target
	glBindTexture(GL_TEXTURE_3D, g_volTexObj);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	if (w > 0 && h > 0 && d > 0)
	{
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, w, h, d, 0, GL_RED, GL_FLOAT, data.data());
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			//std::cout << "OpenGL Error: " << GlErrorToString(error) << std::endl;
		}
	}
	else
	{
		std::cout << "Invalid texture dimensions: w=" << w << ", h=" << h << ", d=" << d << std::endl;
	}

	
	return g_volTexObj;
}

void
init()
{

	_init_framebuf();


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

	// initialize projection matrix
	projection = glm::perspective(glm::radians(45.0f), (float)scrn_width / (float)scrn_height, 0.01f, 10000.0f);

	tex3d = initVol3DTex();
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
	gfx_backend->setClearColor(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
	gfx_backend->clearBuffer();

	ImGui::DragFloat("min", &mind);
	ImGui::DragFloat("max", &maxd);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glFrontFace(GL_CCW);
	//glDisable(GL_SCISSOR_TEST);
	//glDepthFunc(GL_LESS);

	gfx_backend->bindGPUProgram(gpu_program2);

	glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::mat4 view = glm::mat4(1.0f);

	view = camera.getViewMatrix();

	auto mvp = projection * view * model;

	gfx_backend->setGPUProgramMat4(gpu_program2, "model", model);
	gfx_backend->setGPUProgramMat4(gpu_program2, "view", view);
	gfx_backend->setGPUProgramMat4(gpu_program2, "mvp", mvp);


	gfx_backend->setGPUProgramFloat(gpu_program2, "minIntensity", mind);
	gfx_backend->setGPUProgramFloat(gpu_program2, "maxIntensity", maxd);


	auto slow_ray_step = camera.getForwardVector() * scalar;
	
	glUniform3fv(
		glGetUniformLocation(gpu_program2, "ray_step"),
		1,
		glm::value_ptr(glm::vec3(slow_ray_step.x, slow_ray_step.y, slow_ray_step.z)));


	GLint volumeLoc = glGetUniformLocation(gpu_program2, "volume_texture");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, tex3d);

	gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES, gpu_mesh_id2, 36);

	//glDisable(GL_CULL_FACE);
	//glEnable(GL_SCISSOR_TEST);
	frame_buffer->Unbind();
}

void
render()
{
	// render to frame buffer
	render_pass_1();

	// render to main buffer
	gfx_backend->setClearColor(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
	gfx_backend->clearBuffer();

	gfx_backend->bindTexture2D(frame_buffer->GetTexture());
	gfx_backend->bindGPUProgram(gpu_program);

	gfx_backend->draw_indexed(gfx::GFX_Primitive::TRIANGLES, gpu_mesh_id, 6);
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