#include "gfx.h"
#include "gfx_fbo.h"

#include <imgui.h>
#include <iostream>

// global
auto gfx_backend = std::make_shared<gfx::GFX>();

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
	getPosition()
	{
		// Convert spherical coordinates to Cartesian coordinates
		float x = distance * cos(glm::radians(elevation)) * cos(glm::radians(azimuth));
		float y = distance * sin(glm::radians(elevation));
		float z = distance * cos(glm::radians(elevation)) * sin(glm::radians(azimuth));

		// Create the camera position
		glm::vec3 cameraPosition(x, y, z);

		// The view matrix is the inverse of the look-at matrix
		return cameraPosition;
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

class Plane
{
public:
	Plane()
	{

		// clang-format off
		vertices = {
			// positions         // texture coords  // normals
			-1.0f, 0.0f, -1.0f,		0.0f, 0.0f,		0.0f, 1.0f, 0.0f,
			1.0f, 0.0f,  1.0f,		1.0f, 1.0f,		0.0f, 1.0f, 0.0f,
			1.0f, 0.0f, -1.0f,		1.0f, 0.0f,		0.0f, 1.0f, 0.0f,

			-1.0f, 0.0f, -1.0f,		0.0f, 0.0f,		0.0f, 1.0f, 0.0f,
			-1.0f, 0.0f,  1.0f,		0.0f, 1.0f,		0.0f, 1.0f, 0.0f ,
			 1.0f, 0.0f,  1.0f,		1.0f, 1.0f,		0.0f, 1.0f, 0.0f
		};
		// clang-format on

		model = glm::scale(glm::mat4(1.0f), glm::vec3(100, 0, 100));

		vertex_buffer_id = gfx_backend->createVertexBuffer(
			vertices.data(),
			sizeof(vertices[0]) * vertices.size(),
			gfx::BUFFER_USAGE::STATIC);

		gfx::Attributes attributes;
		attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "POSITION"));
		attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC2, "TEXCOORD"));
		attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "NORMAL"));

		gpu_mesh_id = gfx_backend->createGPUMesh(vertex_buffer_id, attributes);

		scale_val = 40.0f;
		color1 = glm::vec3(1.0f, 1.0f, 1.0f);
		color2 = glm::vec3(0.6f, 0.6f, 1.0f);
	}

	~Plane() {}

	glm::mat4 model;
	std::vector<float> vertices;
	uint32_t vertex_buffer_id, gpu_mesh_id;
	float scale_val;
	glm::vec3 color1;
	glm::vec3 color2;
};

class Sphere
{
public:
	Sphere()
	{
		radius = 10;
		pos = glm::vec3(0, radius + 1.0, -5);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(pos));

		generate(radius, 150, 150);

		vertex_buffer_id = gfx_backend->createVertexBuffer(
			vertices.data(),
			sizeof(vertices[0]) * vertices.size(),
			gfx::BUFFER_USAGE::STATIC);

		gfx::Attributes attributes;
		attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "POSITION"));
		attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC2, "TEXCOORD"));
		attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "NORMAL"));

		gpu_mesh_id = gfx_backend->createGPUMesh(vertex_buffer_id, attributes);
	}
	~Sphere() {}

	void
	generate(float radius, int sector_count, int stack_count)
	{
		float x, y, z, xy; // vertex position
		float nx, ny, nz;  // vertex normal
		float length_inv = 1.0f / radius;
		float sector_step = 2.0f * glm::pi<float>() / (float)sector_count;
		float stack_step = glm::pi<float>() / (float)stack_count;
		float sector_angle, stack_angle;

		std::vector<glm::vec3> temp_pos;
		std::vector<glm::vec3> temp_normal;

		// generate sphere points
		// top point as FP doesn't produce one point
		for (size_t j = 0; j <= sector_count; ++j)
		{
			temp_pos.push_back(glm::vec3(0, 0, radius));
			temp_normal.push_back(glm::vec3(0, 0, 1));
		}
		for (size_t i = 1, n = stack_count; i < n; ++i)
		{
			stack_angle = (glm::pi<float>() / 2.0f) - (float)i * stack_step; // starting from pi/2 to -pi/2
			xy = radius * std::cos(stack_angle);
			z = radius * std::sin(stack_angle);

			auto idx = temp_pos.size();
			// add (sectorCount+1) vertices per stack
			// the first and last vertices have same position and normal
			for (size_t j = 0; j < sector_count; ++j)
			{
				sector_angle = j * sector_step;

				// vertex position (x, y, z)
				x = xy * std::cos(sector_angle);
				y = xy * std::sin(sector_angle);

				// normalized vertex normal (nx, ny, nz)
				nx = x * length_inv;
				ny = y * length_inv;
				nz = z * length_inv;

				temp_pos.push_back(glm::vec3(x, y, z));
				temp_normal.push_back(glm::vec3(nx, ny, nz));
			}

			// last vertices same as first
			temp_pos.push_back(temp_pos[idx]);
			temp_normal.push_back(temp_normal[idx]);
		}
		// bottom point as FP doesn't produce one point
		for (size_t j = 0; j <= sector_count; ++j)
		{
			temp_pos.push_back(glm::vec3(0, 0, -radius));
			temp_normal.push_back(glm::vec3(0, 0, -1));
		}

		// construct sphere triangles from sphere points
		size_t k1, k2;
		for (size_t i = 0; i < stack_count; ++i)
		{
			k1 = i * (sector_count + 1); // beginning of current stack
			k2 = k1 + sector_count + 1;	 // beginning of next stack

			for (size_t j = 0; j < sector_count; ++j, ++k1, ++k2)
			{
				// 2 triangles per sector excluding first and last stacks
				// k1 => k2 => k1+1
				if (i != 0)
				{
					// construct triangle
					vertices.push_back(temp_pos[k1].x);
					vertices.push_back(temp_pos[k1].y);
					vertices.push_back(temp_pos[k1].z);

					// push dummy uvs
					vertices.push_back(0.0f);
					vertices.push_back(0.0f);

					vertices.push_back(temp_normal[k1].x);
					vertices.push_back(temp_normal[k1].y);
					vertices.push_back(temp_normal[k1].z);

					vertices.push_back(temp_pos[k2].x);
					vertices.push_back(temp_pos[k2].y);
					vertices.push_back(temp_pos[k2].z);

					// push dummy uvs
					vertices.push_back(0.0f);
					vertices.push_back(0.0f);

					vertices.push_back(temp_normal[k2].x);
					vertices.push_back(temp_normal[k2].y);
					vertices.push_back(temp_normal[k2].z);

					vertices.push_back(temp_pos[k1 + 1].x);
					vertices.push_back(temp_pos[k1 + 1].y);
					vertices.push_back(temp_pos[k1 + 1].z);

					// push dummy uvs
					vertices.push_back(0.0f);
					vertices.push_back(0.0f);

					vertices.push_back(temp_normal[k1 + 1].x);
					vertices.push_back(temp_normal[k1 + 1].y);
					vertices.push_back(temp_normal[k1 + 1].z);
				}

				// k1+1 => k2 => k2+1
				if (i != (stack_count - 1))
				{
					vertices.push_back(temp_pos[k1 + 1].x);
					vertices.push_back(temp_pos[k1 + 1].y);
					vertices.push_back(temp_pos[k1 + 1].z);

					// push dummy uvs
					vertices.push_back(0.0f);
					vertices.push_back(0.0f);

					vertices.push_back(temp_normal[k1 + 1].x);
					vertices.push_back(temp_normal[k1 + 1].y);
					vertices.push_back(temp_normal[k1 + 1].z);

					vertices.push_back(temp_pos[k2].x);
					vertices.push_back(temp_pos[k2].y);
					vertices.push_back(temp_pos[k2].z);

					// push dummy uvs
					vertices.push_back(0.0f);
					vertices.push_back(0.0f);

					vertices.push_back(temp_normal[k2].x);
					vertices.push_back(temp_normal[k2].y);
					vertices.push_back(temp_normal[k2].z);

					vertices.push_back(temp_pos[k2 + 1].x);
					vertices.push_back(temp_pos[k2 + 1].y);
					vertices.push_back(temp_pos[k2 + 1].z);

					// push dummy uvs
					vertices.push_back(0.0f);
					vertices.push_back(0.0f);

					vertices.push_back(temp_normal[k2 + 1].x);
					vertices.push_back(temp_normal[k2 + 1].y);
					vertices.push_back(temp_normal[k2 + 1].z);
				}
			}
		}
	}

	glm::vec3 pos;
	glm::mat4 model;
	float radius;
	uint32_t vertex_buffer_id, gpu_mesh_id;
	std::vector<float> vertices;
};

OrbitalCamera camera(68.0f, -0.1f, 25.0f);

int scrn_width = 800;
int scrn_height = 600;

int shadow_width = 2048;
int shadow_height = 2048;

uint32_t sky_vertex_buffer_id, sky_gpu_mesh_id, sky_gpu_program;
uint32_t scene_vertex_buffer_id, scene_gpu_mesh_id, scene_gpu_program;

uint32_t depth_gpu_program, quad_vertex_buffer_id, quad_index_buffer_id, quad_gpu_mesh_id;
std::shared_ptr<gfx::Framebuffer> depth_frame_buffer;

float near_plane = 0.1f, far_plane = 200.0f;

glm::vec3 light_pos(50.0, 50.0, 50.0);

// create transformations
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

glm::mat4 light_view = glm::mat4(1.0f);
glm::mat4 light_projection = glm::mat4(1.0f);

std::shared_ptr<Plane> scene_plane;
std::shared_ptr<Sphere> sphere;

inline static void
_init_scene()
{
	const char* depth_vertexShader =
		R"(
		#version 450 core
		layout (location = 0) in vec3 aPos;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main()
		{
			gl_Position = projection * view * model * vec4(aPos, 1.0);
		})";

	const char* depth_fragmentShader =
		R"(
		#version 450 core
		out vec4 FragColor;
		void main()
		{
			gl_FragDepth = gl_FragCoord.z;
		})";

	const char* vertexShader =
		R"(
		#version 450 core
		layout(location = 0) in vec3 aPos;
		layout(location = 1) in vec2 aTexCoord;
		layout(location = 2) in vec3 aNormal;

		out vec2 TexCoord; // Output texture coordinates
		out vec3 fragPos;      // Position of the fragment
		out vec3 fragNormal;   // Normal of the fragment
		out vec4 FragPosLightSpace;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;
		uniform mat4 lightSpaceMatrix;

		void main()
		{
			fragPos = (model * vec4(aPos, 1.0)).xyz;
			fragNormal = normalize(transpose(inverse(model)) * vec4(aNormal, 1.0)).xyz;
			TexCoord = aTexCoord;
			FragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);

			gl_Position = projection * view * vec4(fragPos, 1.0);
			
		})";

	const char* fragmentShader =
		R"(
		#version 450 core
		out vec4 FragColor;
		in vec2 TexCoord;
		in vec3 fragNormal;
		in vec3 fragPos;
		in vec4 FragPosLightSpace;

		uniform vec3 lightPos;     // Position of the point light

		uniform bool use_checker_texture;
		uniform float scale; // Adjust this value for larger/smaller squares
		uniform vec3 color1; // white
		uniform vec3 color2; // black

		uniform float gamma;

		uniform sampler2D shadowMap;

		float ShadowCalculation(vec4 fragPosLightSpace) {
			// perform perspective divide
			vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

			// transform to [0,1] range
			projCoords = projCoords * 0.5 + 0.5;

			// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
			float closestDepth = texture(shadowMap, projCoords.xy).r; 

			// get depth of current fragment from light's perspective
			float currentDepth = projCoords.z;

			// calculate bias (based on depth map resolution and slope)
			vec3 normal = normalize(fragNormal);
			vec3 lightDir = normalize(lightPos - fragPos);
			float bias = max(0.1 * (1.0 - dot(normal, lightDir)), 0.01);

			float shadow = 0.0;
			vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
			for(int x = -1; x <= 1; ++x)
			{
				for(int y = -1; y <= 1; ++y)
				{
					float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
					shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
				}    
			}
			shadow /= 9.0;

			// keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
			if(projCoords.z > 1.0)
				shadow = 0.0;

			return shadow;
		}

		vec4 checker(vec2 _TexCoord, float _scale, vec3 _color1, vec3 _color2)
		{
			// Scale the checker pattern
			vec2 pos = _TexCoord * _scale;

			// Create a checker pattern
			float checker = step(0.5, mod(floor(pos.x) + floor(pos.y), 2.0));

			// Calculate the distance to the edges
			float distToEdgeX = abs(fract(pos.x));
			float distToEdgeY = abs(fract(pos.y));
			float edgeDistance = min(distToEdgeX, distToEdgeY);

			// Smooth the transition with a simple step function
			float smoothing = smoothstep(0.0, 0.01, edgeDistance); // Adjust the second parameter for more or less smoothing

			// Mix colors based on the checker pattern and smoothing
			return vec4(mix(_color1, _color2, mix(checker, 1.0 - checker, smoothing)), 1.0);
		}

		vec3 Uncharted2ToneMapping(vec3 color)
		{
			float A = 0.15;
			float B = 0.50;
			float C = 0.10;
			float D = 0.20;
			float E = 0.02;
			float F = 0.30;
			float W = 11.2;
			float exposure = 2.;
			color *= exposure;
			color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
			float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
			color /= white;
			return color;
		}

		void main()
		{
			vec4 final_col = vec4(1.0,1.0,1.0,1.0);

			if(use_checker_texture)
			{
				final_col = checker(TexCoord, scale, color1, color2);
			}
	
			// Normalize the normal vector
			vec3 norm = normalize(fragNormal);

			// Calculate the direction from the fragment to the light
			vec3 lightDir = normalize(lightPos - fragPos);

			// Calculate the distance to the light
			float distance = length(lightPos - fragPos);

			// Calculate the diffuse component
			float diff = max(dot(norm, lightDir), 0.0);
			vec3 diffuse = vec3(1.0) * diff;

			float shad = 1.0;
			vec3 indirect = vec3(0.5,0.5,0.5);
			final_col = final_col * vec4((diffuse*shad+indirect * 0.5), 1.0);

			vec3 tone = Uncharted2ToneMapping((final_col * 2).xyz);
			final_col = vec4(tone, 1.0);

			// skip back faces from shadow calculation
			if (gl_FrontFacing == false)
			{
				FragColor = final_col;
			}
			else
			{
				// calculate shadow
				float shadow = ShadowCalculation(FragPosLightSpace);
				vec3 result = ((1.0 - shadow * 0.7)) * final_col.xyz;
				result = mix(result, vec3(0.7, 0.7, 0.8), 0.2);
				FragColor = vec4(result, 1.0);
			}
		})";

	// clang-format on

	// build and compile our shader program
	scene_gpu_program = gfx_backend->createGPUProgram(vertexShader, fragmentShader);
	depth_gpu_program = gfx_backend->createGPUProgram(depth_vertexShader, depth_fragmentShader);

	scene_plane = std::make_shared<Plane>();
	sphere = std::make_shared<Sphere>();

	light_view = glm::lookAt(light_pos, camera.target, glm::vec3(0, 1, 0));
	light_projection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);

	// clang-format off
	float vertices[] = {
		// positions         // texture coords
		 1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // top right
		 1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // bottom right
		-1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // bottom left
		-1.0f,  1.0f, 0.0f,  0.0f, 1.0f  // top left 
	};

	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	// clang-format on

	quad_vertex_buffer_id = gfx_backend->createVertexBuffer(vertices, sizeof(vertices), gfx::BUFFER_USAGE::STATIC);
	quad_index_buffer_id = gfx_backend->createIndexBuffer(indices, sizeof(indices), gfx::BUFFER_USAGE::STATIC);

	gfx::Attributes attributes;
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "POSITION"));
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC2, "TEXCOORD"));

	quad_gpu_mesh_id = gfx_backend->createGPUMesh(quad_vertex_buffer_id, quad_index_buffer_id, attributes);

	// create another render buffer to render on it.
	depth_frame_buffer =
		std::make_shared<gfx::Framebuffer>(shadow_width, shadow_height, gfx::FrameBuffer_Mode::DepthBuffer);
}

void
render_depthmap()
{

	depth_frame_buffer->Bind();
	gfx_backend->clearBuffer();

	gfx_backend->bindGPUProgram(depth_gpu_program);
	gfx_backend->setGPUProgramMat4(depth_gpu_program, "view", light_view);
	gfx_backend->setGPUProgramMat4(depth_gpu_program, "projection", light_projection);

	gfx_backend->setGPUProgramMat4(depth_gpu_program, "model", scene_plane->model);
	gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES, scene_plane->gpu_mesh_id, scene_plane->vertices.size() / 8);

	gfx_backend->setGPUProgramMat4(depth_gpu_program, "model", sphere->model);
	gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES, sphere->gpu_mesh_id, sphere->vertices.size() / 8);

	depth_frame_buffer->Unbind();
}

inline static void
_draw_scene()
{
	gfx_backend->updateViewport(shadow_width, shadow_height);
	render_depthmap();

	gfx_backend->updateViewport(scrn_width, scrn_height);

	gfx_backend->bindGPUProgram(scene_gpu_program);
	gfx_backend->bindTexture2D(depth_frame_buffer->GetTexture());

	gfx_backend->setGPUProgramMat4(scene_gpu_program, "view", view);
	gfx_backend->setGPUProgramMat4(scene_gpu_program, "projection", projection);
	gfx_backend->setGPUProgramMat4(scene_gpu_program, "lightSpaceMatrix", light_projection * light_view);
	gfx_backend->setGPUProgramVec3(scene_gpu_program, "lightPos", light_pos);

	// render cyclorama
	gfx_backend->setGPUProgramMat4(scene_gpu_program, "model", scene_plane->model);
	gfx_backend->setGPUProgramInt(scene_gpu_program, "use_checker_texture", 1);
	gfx_backend->setGPUProgramFloat(scene_gpu_program, "scale", scene_plane->scale_val);
	gfx_backend->setGPUProgramVec3(scene_gpu_program, "color1", scene_plane->color1);
	gfx_backend->setGPUProgramVec3(scene_gpu_program, "color2", scene_plane->color2);
	gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES, scene_plane->gpu_mesh_id, scene_plane->vertices.size() / 8);

	// render sphere
	gfx_backend->setGPUProgramMat4(scene_gpu_program, "model", sphere->model);
	gfx_backend->setGPUProgramInt(scene_gpu_program, "use_checker_texture", 0);
	gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES, sphere->gpu_mesh_id, sphere->vertices.size() / 8);
}

void
init()
{
	// clang-format off

	const char* sky_vertexShader = R"(
		#version 450 core

		layout (location = 0) in vec3 Position;

		out vec2 v;

		void main()
		{
			v = Position.xy;
			gl_Position = vec4(Position, 1.0);
		})";

		// refrence shader : https://www.shadertoy.com/view/4dl3z7
		const char* sky_fragmentShader =R"(
		#version 450 core
		uniform mat4 inv_view;
		uniform vec2 resolution;
		uniform vec3 lightPos;
		uniform vec3 cameraPos;

		in vec2 v;

		out vec4 FragColor;

		// random/hash function
		float hash(float n)
		{
			return fract(sin(n) * 43758.5453123);
		}

		float noise(vec3 x)
		{
			vec3 f = fract(x);
			float n = dot(floor(x), vec3(1.0, 157.0, 113.0));
			return mix(mix(mix(hash(n +   0.0), hash(n +   1.0), f.x),
							mix(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
						mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
							mix(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
		}

		mat3 m = mat3( 0.00,  1.60,  1.20, -1.60,  0.72, -0.96, -1.20, -0.96,  1.28 );

		// Fractional Brownian motion
		float fbm( vec3 p )
		{
			float f = 0.0;
			f += noise(p) / 2; p = m * p * 1.1;
			f += noise(p) / 4; p = m * p * 1.2;
			f += noise(p) / 6; p = m * p * 1.3;
			f += noise(p) / 12; p = m * p * 1.4;
			f += noise(p) / 24;
			return f;
		}

		vec3 skyColor( in vec3 rd )
		{
			vec3 moondir = normalize( lightPos );

			float yd = min(rd.y, 0.);
			rd.y = max(rd.y, 0.);

			vec3 col = vec3(0.);
			// render sky
			float t = pow(1.0-0.7*rd.y, 1.0);
			col += vec3(.1, .2, .4)*(1.0-t);
			// moon
			col += min( vec3(2.0, 2.0, 2.0), vec3(2.0,2.0,2.0) * pow( max(dot(rd,moondir),0.), 350.0 )) * 0.3;
			// moon haze
			col += 0.6* vec3(0.8,0.9,1.0) * pow( max(dot(rd,moondir),0.), 6.0 );

			// stars
			vec3 stars = vec3(0.,0.,0.);
			vec3 scol = clamp(vec3(1.2, 1.0, 0.8) * pow(noise(rd*120.), 120.) * 50. * (.5-pow(t,20.)), 0.0, 1.0);
			stars = scol * (.3+fbm(rd)) * 5;
			col += stars;

			return col;
		}

		void main()
		{
			vec3 dir = vec3( normalize( inv_view * vec4(v.xy*vec2(resolution.x/resolution.y,1),-1.5,0.0) ) );
			FragColor = vec4(skyColor(dir),1.0);
		})";

	float vertices[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f 
	};
	// clang-format on

	sky_vertex_buffer_id = gfx_backend->createVertexBuffer(vertices, sizeof(vertices), gfx::BUFFER_USAGE::STATIC);

	gfx::Attributes attributes;
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "POSITION"));

	sky_gpu_mesh_id = gfx_backend->createGPUMesh(sky_vertex_buffer_id, attributes);

	// build and compile our shader program
	sky_gpu_program = gfx_backend->createGPUProgram(sky_vertexShader, sky_fragmentShader);

	projection = glm::perspective(glm::radians(45.0f), (float)scrn_width / (float)scrn_height, 0.1f, 1000.0f);

	_init_scene();
}

void
render()
{
	gfx_backend->setClearColor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f));
	gfx_backend->clearBuffer();

	gfx_backend->bindGPUProgram(sky_gpu_program);

	view = camera.getViewMatrix();
	gfx_backend->setGPUProgramMat4(sky_gpu_program, "inv_view", glm::inverse(view));
	gfx_backend->setGPUProgramVec2(sky_gpu_program, "resolution", glm::vec2(scrn_width, scrn_height));
	gfx_backend->setGPUProgramVec3(sky_gpu_program, "lightPos", light_pos);
	gfx_backend->setGPUProgramVec3(sky_gpu_program, "cameraPos", camera.getPosition());

	glDisable(GL_DEPTH_TEST);
	gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES_STRIP, sky_gpu_mesh_id, 4);
	glEnable(GL_DEPTH_TEST);

	_draw_scene();
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
	gfx_backend->init("night directional light example", scrn_width, scrn_height);
	gfx_backend->on_Init(init);
	gfx_backend->on_Render(render);
	gfx_backend->on_Resize(resize);
	gfx_backend->on_MouseScroll(mouse_scroll);
	gfx_backend->on_MouseMove(mouse_move);
	gfx_backend->on_MouseButton(mouse_button);
	gfx_backend->start();

	return 0;
}