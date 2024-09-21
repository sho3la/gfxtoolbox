#include <iostream>

#include "gfx.h"
#include "gfx_fbo.h"

#include <imgui.h>

#include <iostream>
#include <fstream>

// global
auto gfx_backend = std::make_shared<gfx::GFX>();

uint32_t vertex_buffer_id, index_buffer_id, gpu_mesh_id, gpu_program;

uint32_t vertex_buffer_id2, index_buffer_id2, gpu_mesh_id2, gpu_program2;

int scrn_width = 800;
int scrn_height = 600;
std::shared_ptr<gfx::Framebuffer> frame_buffer;

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
			uniform  mat4 mvp;

			void main()
			{
				vec4 cs_position = mvp * vec4(input_position, 1.0);
				vec3 uv = input_position + vec3(0.5);

				gl_Position = cs_position;
				entryPoint = uv; // Pass the entry point
			})";

// Fragment Shader Source
const char* fragmentShaderSource = R"(
		#version 450

		in vec3 entryPoint;
		out vec4 fragColor;

		uniform sampler3D volume; // 3D texture containing DICOM intensity data

		const vec2 screenSize = vec2(800,600); // Size of the screen
		const float minIntensity = -1000; // Minimum intensity value
		const float maxIntensity = 20434; // Maximum intensity value

		void main() {

			// Get exit point from the exitPoints texture
			vec3 exitPoint = vec3(gl_FragCoord.st / screenSize, 0.0);

			// Skip rendering if the entry and exit points are the same
			if (entryPoint == exitPoint)
				discard;

			// Compute the direction from entry point to exit point
			vec3 dir = exitPoint - entryPoint;
			float len = length(dir);
			vec3 dirN = normalize(dir);
			vec3 deltaDir = dirN * 0.05; // Small step size

			vec3 voxelCoord = entryPoint; // Start at the entry point
			vec4 colorAccum = vec4(0.0); // Accumulated color

			// Sample along the ray for a fixed number of iterations
			for (int i = 0; i < 1000; i++)
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
				voxelCoord += deltaDir;

				// Stop if we've reached the exit point
				if (length(voxelCoord - entryPoint) >= len) {
					break;
				}
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

float min = FLT_MAX;
float max = -FLT_MAX;
auto read_img_file = [&](const char* filename) -> std::shared_ptr<gfx::Image3D> {
	std::ifstream infile(filename);
	if (!infile)
	{
		std::cerr << "Error opening file for reading: " << filename << std::endl;
	}

	int width, height, depth;
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

		width = std::stof(words[0]);
		height = std::stof(words[1]);
		depth = std::stof(words[2]);
	}

	auto image = std::make_shared<gfx::Image3D>(width, height, depth);

	std::vector<float> data;
	data.reserve(width* height* depth);


	
	while (std::getline(infile, line))
	{
		auto val = std::stof(line);
		//std::cout << val << std::endl;
		if (val > max)
		{
			max = val;
		}

		if (val < min)
		{
			min = val;
		}
		data.push_back(val);
	}

	infile.close();

	image->setData(data);

	return image;
};

// init 3D texture to store the volume data used fo ray casting
GLuint
initVol3DTex()
{
	unsigned int g_volTexObj;
	auto img = read_img_file(DATA_DIR "img_3d.txt");
	auto w = img->getWidth();
	auto h = img->getHeight();
	auto d = img->getDepth();

	glGenTextures(1, &g_volTexObj);

	// bind 3D texture target
	glBindTexture(GL_TEXTURE_3D, g_volTexObj);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// pixel transfer happens here from client to OpenGL server
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if (w > 0 && h > 0 && d > 0)
	{
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, w, h, d, 0, GL_RED, GL_FLOAT, img->getData().data());
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
	projection = glm::perspective(glm::radians(45.0f), (float)scrn_width / (float)scrn_height, 0.1f, 100.0f);

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
}

void
render_pass_1()
{

	frame_buffer->Bind();
	gfx_backend->setClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	gfx_backend->clearBuffer();

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glFrontFace(GL_CCW);
	//glDisable(GL_SCISSOR_TEST);
	//glDepthFunc(GL_LESS);

	gfx_backend->bindGPUProgram(gpu_program2);

	glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::mat4 view = glm::mat4(1.0f);

	model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

	auto mvp = projection * view * model;

	gfx_backend->setGPUProgramMat4(gpu_program2, "model", model);
	gfx_backend->setGPUProgramMat4(gpu_program2, "mvp", mvp);
	

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