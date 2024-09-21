#include <iostream>

#include "gfx.h"
#include "gfx_fbo.h"

#include <imgui.h>

#include <fstream>

// global
auto gfx_backend = std::make_shared<gfx::GFX>();

uint32_t vertex_buffer_id, index_buffer_id, gpu_mesh_id, gpu_program;

uint32_t vertex_buffer_id2, index_buffer_id2, gpu_mesh_id2, gpu_program2, gpu_program3;

uint32_t tex1d, tex3d, gradient3d;

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


// Vertex Shader Source
const char* raycast_vshader = R"(
		#version 450 core
		layout(location = 0) in vec3 vVertex;

		uniform mat4 mvp;

		out vec3 entryPoint;

		void main()
		{
			gl_Position =  mvp * vec4(vVertex,1);
			entryPoint = vVertex;
		})";

// Fragment Shader Source
const char* raycast_fshader = R"(
		#version 450 core
		in vec3 entryPoint;

		uniform vec2 screenSize;
		uniform sampler2D exitPoints;
		uniform sampler3D volume;
		uniform sampler1D transferFn;
		uniform sampler3D gradients;
		uniform mat4 viewMat;
		uniform mat4 normalMat;

		out vec4 fragColor;

		const int MAX_ITERATIONS = 2000;
		const float stepSize = 0.005;
		const float specularExp = 128.0;
		const float ambientLight = 0.2;
		const vec4 bgColor = vec4(0.2, 0.2, 0.2, 1.0);
		const vec3 lightPositionWorld = vec3(10.0, 5.0, -10.0);

		// light constants
		const vec3 Ls = vec3(1.0, 1.0, 1.0);
		const vec3 Ld = vec3(1.0, 1.0, 1.0);
		const vec3 La = vec3(0.1, 0.1, 0.1);

		// surface reflectance
		const vec3 Ks = vec3(1.0, 1.0, 1.0);    // fully reflect specular light
		const vec3 Kd = vec3(0.1, 0.1, 0.1);    // orange diffuse surface reflectance
		const vec3 Ka = vec3(1.0, 1.0, 1.0);    // fully reflect ambient light
		const vec3 Ia = La * Ka;                // ambient intensity
		const float specularExponent = 100.0;   // specular 'power'

		// diffuse intensity
		vec3 diffuse(vec3 color, vec3 normal, vec3 directionToLight) {
			return Ld * color * max(0, dot(normal, directionToLight));
		}

		// specular intensity
		vec3 specular(vec3 viewSpacePosition, vec3 viewSpaceLightPosition, vec3 normal, vec3 directionToLight) {
			vec3 reflectionEye = reflect(-directionToLight, normal);
			vec3 surfaceToViewerEye = normalize(-viewSpacePosition);
			float dotProdSpecular = max(dot(reflectionEye, surfaceToViewerEye), 0.0);
			float specular_factor = pow (dotProdSpecular, specularExponent);
			return Ls * Ks * specular_factor;
		}

		// phong lightning
		vec3 phong(vec3 color, vec3 normal, vec3 viewSpacePosition, vec3 viewSpaceLightPosition)
		{
		  vec3 directionToLight = normalize(viewSpaceLightPosition - viewSpacePosition);
		  vec3 Id = diffuse(color, normal, directionToLight);
		  vec3 Is = specular(viewSpacePosition, viewSpaceLightPosition, normal, directionToLight);
		  return Is + Id + Ia;
		}

		float random(vec2 cords) {
			return 1 * fract(sin(cords.x * 12.9898 + cords.y * 78.233) * 43758.5453);
		}

		// depth is calculated from viewing direction
		vec3 calcVoxel(vec3 point, vec2 cords, vec3 deltaDir) {
			return point + deltaDir * random(cords);
		}


		void main()
		{
			vec3 exitPoint = texture(exitPoints, gl_FragCoord.st / screenSize).xyz;

			// empty space skipping
			if (entryPoint == exitPoint)
				discard;

			vec3 dir = (exitPoint - entryPoint);
			float len = length(dir);
			vec3 dirN = normalize(dir);
			vec3 deltaDir = dirN * stepSize;
			float deltaDirLen = length(deltaDir);

			vec3 voxelCoord = calcVoxel(entryPoint, gl_FragCoord.xy, deltaDir);

			float alphaAcum = 0.0;
			float lengthAcum = 0.0;
			vec4 colorAcum = vec4(0.0);

			float intensity;
			vec4 sampleColor;
			vec3 gradient;

			vec3 viewSpacePosition;
			vec3 viewSpaceNormal;
			vec3 viewSpaceLightPosition = vec3(viewMat * vec4(lightPositionWorld, 1.0));

			for(int i = 0; i < MAX_ITERATIONS; i++)
			{
    			intensity = texture(volume, voxelCoord).x;
				gradient = normalize(texture(gradients, voxelCoord).xyz);
				sampleColor = texture(transferFn, intensity);

				viewSpacePosition = voxelCoord;
				viewSpaceNormal = normalize((normalMat * vec4(gradient, 0.0)).xyz);

				if (sampleColor.a > 0.0) {
					// correction
    				sampleColor.a = 1.0 - pow(1.0 - sampleColor.a, stepSize * 100.0f);
					sampleColor.rbg = phong(sampleColor.rgb, viewSpaceNormal, viewSpacePosition, viewSpaceLightPosition);

					colorAcum.rgb = colorAcum.rbg + (1.0 - colorAcum.a) * sampleColor.rgb * sampleColor.a ;
					colorAcum.a = colorAcum.a + (1.0 - colorAcum.a) * sampleColor.a;
    			}

				// increment voxel along viewing direction
				voxelCoord += deltaDir;
    			lengthAcum += deltaDirLen;

				if (lengthAcum >= len ) {
    				colorAcum.rgb = colorAcum.rgb*colorAcum.a + (1 - colorAcum.a) * bgColor.rgb;
    				break;
    			}else if (colorAcum.a > 1.0) {
    				colorAcum.a = 1.0;
    				break;
    			}
			}
			fragColor = colorAcum;
		})";

// create transformations
glm::mat4 projection = glm::mat4(1.0f);

#include <array>
#include <cstdint>
#include <stdexcept>

class TransferFunction
{

public:
	static const int SIZE = 256;
	TransferFunction()
	{
		for (int i = 0; i < SIZE; ++i)
		{
			func[i] = i;
		}
	}

	std::vector<uint8_t>
	buffer()
	{
		std::vector<uint8_t> rgba(SIZE * 4); // 4 bytes per pixel (RGBA)
		auto ptr = rgba.data();

		for (int b : func)
		{
			tf3(ptr, b);
			ptr += 4; // Move to the next pixel
		}

		return rgba;
	}

private:
	std::array<int, SIZE> func;

	void
	tf1(uint8_t* ptr, int b)
	{
		ptr[0] = static_cast<uint8_t>(b); // r
		ptr[1] = static_cast<uint8_t>(b); // g
		ptr[2] = static_cast<uint8_t>(b); // b
		ptr[3] = static_cast<uint8_t>(b); // a
	}

	void
	tf2(uint8_t* ptr, int b)
	{
		ptr[0] = static_cast<uint8_t>(b);						   // r
		ptr[1] = static_cast<uint8_t>(b);						   // g
		ptr[2] = static_cast<uint8_t>(b);						   // b
		ptr[3] = (b < 50 || b > 60) ? 0 : static_cast<uint8_t>(b); // a
	}

	void
	tf3(uint8_t* ptr, int b)
	{
		if (b >= 41 && b <= 64)
		{
			ptr[0] = 220;					  // r
			ptr[1] = 100;					  // g
			ptr[2] = 0;						  // b
			ptr[3] = static_cast<uint8_t>(b); // a
		}
		else if (b > 100)
		{
			ptr[0] = 220; // r
			ptr[1] = 220; // g
			ptr[2] = 220; // b
			ptr[3] = 200; // a
		}
		else
		{
			ptr[0] = 0; // r
			ptr[1] = 0; // g
			ptr[2] = 0; // b
			ptr[3] = 0; // a
		}
	}
};


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

inline static uint32_t
_tf_texture1d()
{
	TransferFunction* tsf = new TransferFunction();
	auto data = tsf->buffer();


	GLuint id = -1;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_1D, id);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, tsf->SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

	glBindTexture(GL_TEXTURE_1D, 0);

	delete tsf;
	return id;
}


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
	while (std::getline(infile, line))
	{
		data.push_back(std::stof(line));
	}

	infile.close();

	image->setData(data);

	return image;
};



inline static uint32_t
_vol_texture3d(std::shared_ptr<gfx::Image3D>& img)
{
	GLuint id = -1;

	glGenTextures(1, &id);

	glBindTexture(GL_TEXTURE_3D, id);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage3D(
		GL_TEXTURE_3D,
		0,
		GL_RED,
		img->getWidth(),
		img->getHeight(),
		img->getDepth(),
		0,
		GL_RED,
		GL_FLOAT,
		img->getData().data());

	glGenerateMipmap(GL_TEXTURE_3D);

	glBindTexture(GL_TEXTURE_3D, 0);

	return id;
	return id;
}

inline static uint32_t
_gradient_texture3d(std::shared_ptr<gfx::Image3D>& img)
{
	auto gradient = img->gradientsBuffer();
	GLuint id = -1;

	glGenTextures(1, &id);

	glBindTexture(GL_TEXTURE_3D, id);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage3D(
		GL_TEXTURE_3D,
		0,
		GL_RGB,
		img->getWidth(),
		img->getHeight(),
		img->getDepth(),
		0,
		GL_RGB,
		GL_FLOAT,
		gradient.data());

	glGenerateMipmap(GL_TEXTURE_3D);

	glBindTexture(GL_TEXTURE_3D, 0);

	return id;
	return id;
}

inline static void
_raycast_init()
{
	auto img = read_img_file(DATA_DIR "img_3d.txt");

	// load 1d texture transfer function
	tex1d = _tf_texture1d();

	// load 3d texture
	tex3d = _vol_texture3d(img);

	// gradient texture ??
	gradient3d = _gradient_texture3d(img);

	gpu_program3 = gfx_backend->createGPUProgram(raycast_vshader, raycast_fshader);
}

void
init()
{
	_frame_buffer_init();

	_cube_scene_init();

	_raycast_init();

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

	glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::mat4 view = glm::mat4(1.0f);

	model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

	auto mvp = projection * view * model;

	gfx_backend->bindGPUProgram(gpu_program2);
	gfx_backend->setGPUProgramMat4(gpu_program2, "mvp", mvp);
	gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES, gpu_mesh_id2, 36);

	frame_buffer->Unbind();

		/// raycast call

	gfx_backend->setClearColor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f));
	gfx_backend->clearBuffer();

	gfx_backend->bindGPUProgram(gpu_program3);
	gfx_backend->setGPUProgramMat4(gpu_program3, "mvp", mvp);
	gfx_backend->setGPUProgramMat4(gpu_program3, "viewMat", view);
	glUniform2f(glGetUniformLocation(gpu_program3, "screenSize"), scrn_width, scrn_height);

	glm::mat4 combinedMatrix = view * model;
	glm::mat4 invertedMatrix = glm::inverse(combinedMatrix);
	glm::mat4 transposedMatrix = glm::transpose(invertedMatrix);
	gfx_backend->setGPUProgramMat4(gpu_program3, "normalMat", transposedMatrix);

	glActiveTexture(GL_TEXTURE0);
	gfx_backend->bindTexture1D(tex1d);
	glUniform1i(glGetUniformLocation(gpu_program3, "transferFn"), 0);

	glActiveTexture(GL_TEXTURE1);
	gfx_backend->bindTexture2D(frame_buffer->GetTexture());
	glUniform1i(glGetUniformLocation(gpu_program3, "exitPoints"), 1);

	glActiveTexture(GL_TEXTURE2);
	gfx_backend->bindTexture3D(tex3d);
	glUniform1i(glGetUniformLocation(gpu_program3, "volume"), 2);

	glActiveTexture(GL_TEXTURE3);
	gfx_backend->bindTexture3D(gradient3d);
	glUniform1i(glGetUniformLocation(gpu_program3, "gradients"), 3);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES, gpu_mesh_id2, 36);
	glDisable(GL_CULL_FACE);
}

void
render()
{
	// render to frame buffer
	render_pass_1();
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