#include "gfx.h"

#include <imgui.h>
#include <iostream>

// global
auto gfx_backend = std::make_shared<gfx::GFX>();

int scrn_width = 800;
int scrn_height = 600;

// create transformations
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

glm::vec3 cameraPosition(0, 14, 34.5);
glm::vec3 cameraTarget(0, 7.3f, 0);

uint32_t gpu_program;

glm::vec3 point_light_pos(20, 50, 9);
float point_light_power = 1000;

// clang-format off

const char*	vertexShader =
		R"(
		#version 450 core
		layout(location = 0) in vec3 aPos;
		layout(location = 1) in vec2 aTexCoord;
		layout(location = 2) in vec3 aNormal;

		out vec2 TexCoord; // Output texture coordinates
		out vec3 fragPos;      // Position of the fragment
		out vec3 fragNormal;   // Normal of the fragment

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main()
		{
			fragPos = (model * vec4(aPos, 1.0)).xyz;
			fragNormal = normalize(transpose(inverse(model)) * vec4(aNormal, 1.0)).xyz;
			TexCoord = aTexCoord;

			gl_Position = projection * view * vec4(fragPos, 1.0);
			
		})";

const char* fragmentShader =
		R"(
		#version 450 core
		out vec4 FragColor;
		in vec2 TexCoord;
		in vec3 fragNormal;
		in vec3 fragPos;

		uniform vec3 lightPos;     // Position of the point light
		uniform vec3 viewPos;      // Position of the viewer/camera
		uniform vec3 lightColor;   // Color of the light
		uniform float lightPower;  // Power of the light

		uniform bool use_checker_texture;
		uniform float scale; // Adjust this value for larger/smaller squares
		uniform vec3 color1; // white
		uniform vec3 color2; // black

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
    
				// Attenuation calculation
				float attenuation = lightPower / (distance * distance);
				attenuation = clamp(attenuation, 0.0, 1.0); // Clamp to [0, 1] range
    
				// Calculate the diffuse component
				float diff = max(dot(norm, lightDir), 0.0);
    
				// Calculate the view direction
				vec3 viewDir = normalize(viewPos - fragPos);
    
				// Calculate the reflection vector
				vec3 reflectDir = reflect(-lightDir, norm);
    
				// Calculate the specular component
				float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // Shininess factor

				// Ambient light component
				vec3 ambient = 0.1 * lightColor; // Low ambient intensity
    
				// Combine results with attenuation
				vec3 diffuse = diff * lightColor * attenuation;
				vec3 specular = spec * lightColor * attenuation;

				// Final color
				vec3 result = (ambient + diffuse /*+ specular */) * final_col.xyz;

				FragColor = vec4(result, 1.0);
		})";

// clang-format on

class Cyclorama
{
public:
	Cyclorama()
	{

		// clang-format off

		vertices = {
			// positions                       // texture coords              // normals
		11.045807, 0.00146, -8.080369,             0.62773, 0.515268,          -0, 0.9998, 0.019,
		-11.045931, 0.00146, -8.08037,             0.000117, 0.515268,          -0, 0.9998, 0.019,
		-11.044489, 1e-06, 10.054064,             0.000158, 7.9e-05,          -0, 1, 0.0001,
		11.045807, 0.00146, -8.080369,             0.62773, 0.515268,          -0, 0.9998, 0.019,
		-11.044489, 1e-06, 10.054064,             0.000158, 7.9e-05,          -0, 1, 0.0001,
		11.047249, 1e-06, 10.054064,             0.627771, 7.9e-05,          -0, 1, 0.0001,
		-11.045931, 0.00146, -8.08037,             0.000117, 0.515268,          -0, 0.9998, 0.019,
		11.045807, 0.00146, -8.080369,             0.62773, 0.515268,          -0, 0.9998, 0.019,
		11.045708, 0.008849, -8.275193,             0.627728, 0.520807,          -0, 0.9972, 0.0752,
		-11.045931, 0.00146, -8.08037,             0.000117, 0.515268,          -0, 0.9998, 0.019,
		11.045708, 0.008849, -8.275193,             0.627728, 0.520807,          -0, 0.9972, 0.0752,
		-11.046028, 0.008849, -8.275193,             0.000114, 0.520807,          -0, 0.9972, 0.0752,
		-11.046028, 0.008849, -8.275193,             0.000114, 0.520807,          -0, 0.9972, 0.0752,
		11.045708, 0.008849, -8.275193,             0.627728, 0.520807,          -0, 0.9972, 0.0752,
		11.04561, 0.030777, -8.46893,             0.627725, 0.526346,          -0, 0.9888, 0.1495,
		-11.046028, 0.008849, -8.275193,             0.000114, 0.520807,          -0, 0.9972, 0.0752,
		11.04561, 0.030777, -8.46893,             0.627725, 0.526346,          -0, 0.9888, 0.1495,
		-11.046127, 0.030777, -8.46893,             0.000111, 0.526346,          -0, 0.9888, 0.1495,
		-11.046127, 0.030777, -8.46893,             0.000111, 0.526346,          -0, 0.9888, 0.1495,
		11.04561, 0.030777, -8.46893,             0.627725, 0.526346,          -0, 0.9888, 0.1495,
		11.045513, 0.067121, -8.660492,             0.627722, 0.531885,          -0, 0.9748, 0.223,
		-11.046127, 0.030777, -8.46893,             0.000111, 0.526346,          -0, 0.9888, 0.1495,
		11.045513, 0.067121, -8.660492,             0.627722, 0.531885,          -0, 0.9748, 0.223,
		-11.046225, 0.067121, -8.660492,             0.000108, 0.531885,          -0, 0.9748, 0.223,
		-11.046225, 0.067121, -8.660492,             0.000108, 0.531885,          -0, 0.9748, 0.223,
		11.045513, 0.067121, -8.660492,             0.627722, 0.531885,          -0, 0.9748, 0.223,
		11.045417, 0.117676, -8.848809,             0.627719, 0.537425,          -0, 0.9554, 0.2952,
		-11.046225, 0.067121, -8.660492,             0.000108, 0.531885,          -0, 0.9748, 0.223,
		11.045417, 0.117676, -8.848809,             0.627719, 0.537425,          -0, 0.9554, 0.2952,
		-11.046318, 0.117676, -8.848811,             0.000106, 0.537425,          -0, 0.9554, 0.2952,
		-11.046318, 0.117676, -8.848811,             0.000106, 0.537425,          -0, 0.9554, 0.2952,
		11.045417, 0.117676, -8.848809,             0.627719, 0.537425,          -0, 0.9554, 0.2952,
		11.045324, 0.182163, -9.032831,             0.627717, 0.542965,          -0, 0.9307, 0.3657,
		-11.046318, 0.117676, -8.848811,             0.000106, 0.537425,          -0, 0.9554, 0.2952,
		11.045324, 0.182163, -9.032831,             0.627717, 0.542965,          -0, 0.9307, 0.3657,
		-11.046412, 0.182163, -9.032831,             0.000103, 0.542965,          -0, 0.9307, 0.3658,
		-11.046412, 0.182163, -9.032831,             0.000103, 0.542965,          -0, 0.9307, 0.3658,
		11.045324, 0.182163, -9.032831,             0.627717, 0.542965,          -0, 0.9307, 0.3657,
		11.045234, 0.260217, -9.211525,             0.627714, 0.548504,          -0, 0.9008, 0.4342,
		-11.046412, 0.182163, -9.032831,             0.000103, 0.542965,          -0, 0.9307, 0.3658,
		11.045234, 0.260217, -9.211525,             0.627714, 0.548504,          -0, 0.9008, 0.4342,
		-11.046502, 0.260217, -9.211525,             0.0001, 0.548504,          -0, 0.9008, 0.4343,
		-11.046502, 0.260217, -9.211525,             0.0001, 0.548504,          -0, 0.9008, 0.4343,
		11.045234, 0.260217, -9.211525,             0.627714, 0.548504,          -0, 0.9008, 0.4342,
		11.045148, 0.351405, -9.383892,             0.627712, 0.554044,          -0, 0.8658, 0.5003,
		-11.046502, 0.260217, -9.211525,             0.0001, 0.548504,          -0, 0.9008, 0.4343,
		11.045148, 0.351405, -9.383892,             0.627712, 0.554044,          -0, 0.8658, 0.5003,
		-11.046589, 0.351405, -9.383893,             9.8e-05, 0.554044,          -0, 0.8658, 0.5003,
		-11.046589, 0.351405, -9.383893,             9.8e-05, 0.554044,          -0, 0.8658, 0.5003,
		11.045148, 0.351405, -9.383892,             0.627712, 0.554044,          -0, 0.8658, 0.5003,
		11.045064, 0.455215, -9.548971,             0.627709, 0.559584,          -0, 0.826, 0.5636,
		-11.046589, 0.351405, -9.383893,             9.8e-05, 0.554044,          -0, 0.8658, 0.5003,
		11.045064, 0.455215, -9.548971,             0.627709, 0.559584,          -0, 0.826, 0.5636,
		-11.046674, 0.455215, -9.548971,             9.6e-05, 0.559584,          -0, 0.826, 0.5636,
		-11.046674, 0.455215, -9.548971,             9.6e-05, 0.559584,          -0, 0.826, 0.5636,
		11.045064, 0.455215, -9.548971,             0.627709, 0.559584,          -0, 0.826, 0.5636,
		11.044985, 0.571067, -9.705836,             0.627707, 0.565124,          -0, 0.7816, 0.6237,
		-11.046674, 0.455215, -9.548971,             9.6e-05, 0.559584,          -0, 0.826, 0.5636,
		11.044985, 0.571067, -9.705836,             0.627707, 0.565124,          -0, 0.7816, 0.6237,
		-11.046753, 0.571067, -9.705836,             9.3e-05, 0.565124,          -0, 0.7816, 0.6237,
		-11.046753, 0.571067, -9.705836,             9.3e-05, 0.565124,          -0, 0.7816, 0.6237,
		11.044985, 0.571067, -9.705836,             0.627707, 0.565124,          -0, 0.7816, 0.6237,
		11.044909, 0.698314, -9.853611,             0.627705, 0.570664,          -0, 0.7329, 0.6804,
		-11.046753, 0.571067, -9.705836,             9.3e-05, 0.565124,          -0, 0.7816, 0.6237,
		11.044909, 0.698314, -9.853611,             0.627705, 0.570664,          -0, 0.7329, 0.6804,
		-11.046826, 0.698314, -9.853611,             9.1e-05, 0.570664,          -0, 0.7329, 0.6804,
		-11.046826, 0.698314, -9.853611,             9.1e-05, 0.570664,          -0, 0.7329, 0.6804,
		11.044909, 0.698314, -9.853611,             0.627705, 0.570664,          -0, 0.7329, 0.6804,
		11.04484, 0.836243, -9.99147,             0.627703, 0.576205,          -0, 0.68, 0.7332,
		-11.046826, 0.698314, -9.853611,             9.1e-05, 0.570664,          -0, 0.7329, 0.6804,
		11.04484, 0.836243, -9.99147,             0.627703, 0.576205,          -0, 0.68, 0.7332,
		-11.046896, 0.836243, -9.99147,             8.9e-05, 0.576205,          -0, 0.68, 0.7332,
		-11.046896, 0.836243, -9.99147,             8.9e-05, 0.576205,          -0, 0.68, 0.7332,
		11.04484, 0.836243, -9.99147,             0.627703, 0.576205,          -0, 0.68, 0.7332,
		11.044775, 0.984083, -10.118642,             0.627701, 0.581745,          -0, 0.6233, 0.782,
		-11.046896, 0.836243, -9.99147,             8.9e-05, 0.576205,          -0, 0.68, 0.7332,
		11.044775, 0.984083, -10.118642,             0.627701, 0.581745,          -0, 0.6233, 0.782,
		-11.046961, 0.984083, -10.118642,             8.7e-05, 0.581745,          -0, 0.6233, 0.782,
		-11.046961, 0.984083, -10.118642,             8.7e-05, 0.581745,          -0, 0.6233, 0.782,
		11.044775, 0.984083, -10.118642,             0.627701, 0.581745,          -0, 0.6233, 0.782,
		11.044716, 1.141008, -10.234413,             0.627699, 0.587285,          -0, 0.5632, 0.8263,
		-11.046961, 0.984083, -10.118642,             8.7e-05, 0.581745,          -0, 0.6233, 0.782,
		11.044716, 1.141008, -10.234413,             0.627699, 0.587285,          -0, 0.5632, 0.8263,
		-11.04702, 1.141008, -10.234413,             8.6e-05, 0.587285,          -0, 0.5632, 0.8263,
		-11.04702, 1.141008, -10.234413,             8.6e-05, 0.587285,          -0, 0.5632, 0.8263,
		11.044716, 1.141008, -10.234413,             0.627699, 0.587285,          -0, 0.5632, 0.8263,
		11.044663, 1.306138, -10.338138,             0.627698, 0.592825,          -0, 0.4999, 0.8661,
		-11.04702, 1.141008, -10.234413,             8.6e-05, 0.587285,          -0, 0.5632, 0.8263,
		11.044663, 1.306138, -10.338138,             0.627698, 0.592825,          -0, 0.4999, 0.8661,
		-11.047071, 1.306138, -10.338138,             8.4e-05, 0.592825,          -0, 0.4999, 0.8661,
		-11.047071, 1.306138, -10.338138,             8.4e-05, 0.592825,          -0, 0.4999, 0.8661,
		11.044663, 1.306138, -10.338138,             0.627698, 0.592825,          -0, 0.4999, 0.8661,
		11.044619, 1.478553, -10.429237,             0.627697, 0.598365,          -0, 0.4338, 0.901,
		-11.047071, 1.306138, -10.338138,             8.4e-05, 0.592825,          -0, 0.4999, 0.8661,
		11.044619, 1.478553, -10.429237,             0.627697, 0.598365,          -0, 0.4338, 0.901,
		-11.047117, 1.478553, -10.429237,             8.3e-05, 0.598365,          -0, 0.4338, 0.901,
		-11.047117, 1.478553, -10.429237,             8.3e-05, 0.598365,          -0, 0.4338, 0.901,
		11.044619, 1.478553, -10.429237,             0.627697, 0.598365,          -0, 0.4338, 0.901,
		11.04458, 1.657287, -10.5072,             0.627696, 0.603904,          -0, 0.3653, 0.9309,
		-11.047117, 1.478553, -10.429237,             8.3e-05, 0.598365,          -0, 0.4338, 0.901,
		11.04458, 1.657287, -10.5072,             0.627696, 0.603904,          -0, 0.3653, 0.9309,
		-11.047157, 1.657287, -10.5072,             8.2e-05, 0.603905,          -0, 0.3653, 0.9309,
		-11.047157, 1.657287, -10.5072,             8.2e-05, 0.603905,          -0, 0.3653, 0.9309,
		11.04458, 1.657287, -10.5072,             0.627696, 0.603904,          -0, 0.3653, 0.9309,
		11.044546, 1.84134, -10.571593,             0.627695, 0.609444,          -0, 0.2947, 0.9556,
		-11.047157, 1.657287, -10.5072,             8.2e-05, 0.603905,          -0, 0.3653, 0.9309,
		11.044546, 1.84134, -10.571593,             0.627695, 0.609444,          -0, 0.2947, 0.9556,
		-11.04719, 1.84134, -10.571592,             8.1e-05, 0.609444,          -0, 0.2947, 0.9556,
		-11.04719, 1.84134, -10.571592,             8.1e-05, 0.609444,          -0, 0.2947, 0.9556,
		11.044546, 1.84134, -10.571593,             0.627695, 0.609444,          -0, 0.2947, 0.9556,
		11.044521, 2.029685, -10.622053,             0.627694, 0.614984,          -0, 0.2225, 0.9749,
		-11.04719, 1.84134, -10.571592,             8.1e-05, 0.609444,          -0, 0.2947, 0.9556,
		11.044521, 2.029685, -10.622053,             0.627694, 0.614984,          -0, 0.2225, 0.9749,
		-11.047215, 2.029685, -10.622053,             8e-05, 0.614984,          -0, 0.2225, 0.9749,
		-11.047215, 2.029685, -10.622053,             8e-05, 0.614984,          -0, 0.2225, 0.9749,
		11.044521, 2.029685, -10.622053,             0.627694, 0.614984,          -0, 0.2225, 0.9749,
		11.044501, 2.221266, -10.658298,             0.627693, 0.620523,          -0, 0.149, 0.9888,
		-11.047215, 2.029685, -10.622053,             8e-05, 0.614984,          -0, 0.2225, 0.9749,
		11.044501, 2.221266, -10.658298,             0.627693, 0.620523,          -0, 0.149, 0.9888,
		-11.047235, 2.221266, -10.658298,             8e-05, 0.620523,          -0, 0.149, 0.9888,
		-11.047235, 2.221266, -10.658298,             8e-05, 0.620523,          -0, 0.149, 0.9888,
		11.044501, 2.221266, -10.658298,             0.627693, 0.620523,          -0, 0.149, 0.9888,
		11.044491, 2.415012, -10.680127,             0.627693, 0.626062,          -0, 0.0747, 0.9972,
		-11.047235, 2.221266, -10.658298,             8e-05, 0.620523,          -0, 0.149, 0.9888,
		11.044491, 2.415012, -10.680127,             0.627693, 0.626062,          -0, 0.0747, 0.9972,
		-11.047244, 2.415012, -10.680127,             7.9e-05, 0.626062,          -0, 0.0747, 0.9972,
		-11.047244, 2.415012, -10.680127,             7.9e-05, 0.626062,          -0, 0.0747, 0.9972,
		11.044491, 2.415012, -10.680127,             0.627693, 0.626062,          -0, 0.0747, 0.9972,
		11.044487, 2.609841, -10.687416,             0.627693, 0.631601,          -0, 0.0187, 0.9998,
		-11.047244, 2.415012, -10.680127,             7.9e-05, 0.626062,          -0, 0.0747, 0.9972,
		11.044487, 2.609841, -10.687416,             0.627693, 0.631601,          -0, 0.0187, 0.9998,
		-11.047248, 2.609841, -10.687415,             7.9e-05, 0.631601,          -0, 0.0187, 0.9998,
		-11.047248, 2.609841, -10.687415,             7.9e-05, 0.631601,          -0, 0.0187, 0.9998,
		11.044487, 2.609841, -10.687416,             0.627693, 0.631601,          -0, 0.0187, 0.9998,
		11.044487, 15.574547, -10.687419,             0.627693, 0.999921,          -0, -0, 1,
		-11.047248, 2.609841, -10.687415,             7.9e-05, 0.631601,          -0, 0.0187, 0.9998,
		11.044487, 15.574547, -10.687419,             0.627693, 0.999921,          -0, -0, 1,
		-11.047248, 15.574547, -10.687419,             7.9e-05, 0.999921,          -0, -0, 1,
		};

		// clang-format on

		model = glm::scale(glm::mat4(1.0f), glm::vec3(10, 10, 10));

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
		color2 = glm::vec3(0.8f, 0.8f, 0.8f);
	}

	~Cyclorama() {}

	void
	draw()
	{
		gfx_backend->setGPUProgramMat4(gpu_program, "model", model);

		gfx_backend->setGPUProgramInt(gpu_program, "use_checker_texture", 1);
		gfx_backend->setGPUProgramFloat(gpu_program, "scale", scale_val);
		gfx_backend->setGPUProgramVec3(gpu_program, "color1", color1);
		gfx_backend->setGPUProgramVec3(gpu_program, "color2", color2);

		gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES, gpu_mesh_id, vertices.size() / 8);
	}

private:
	glm::mat4 model;
	float scale_val;
	glm::vec3 color1;
	glm::vec3 color2;
	std::vector<float> vertices;
	uint32_t vertex_buffer_id, gpu_mesh_id;
};

class Sphere
{
public:
	Sphere()
	{
		radius = 10;
		pos = glm::vec3(0, radius, -5);
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

	void
	draw()
	{
		gfx_backend->setGPUProgramMat4(gpu_program, "model", model);
		gfx_backend->setGPUProgramInt(gpu_program, "use_checker_texture", 0);
		gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES, gpu_mesh_id, vertices.size() / 8);
	}

	glm::vec3 pos;
	glm::mat4 model;

private:
	float radius;
	uint32_t vertex_buffer_id, gpu_mesh_id;
	std::vector<float> vertices;
};

std::shared_ptr<Cyclorama> scene_cyclorama;
std::shared_ptr<Sphere> sphere;

void
init()
{
	// build and compile our shader program
	gpu_program = gfx_backend->createGPUProgram(vertexShader, fragmentShader);

	scene_cyclorama = std::make_shared<Cyclorama>();
	sphere = std::make_shared<Sphere>();

	// initialize projection matrix
	projection = glm::perspective(glm::radians(45.0f), (float)scrn_width / (float)scrn_height, 0.1f, 10000.0f);
	view = glm::lookAt(cameraPosition, cameraTarget, glm::vec3(0, 1, 0));
}

void
render()
{
	gfx_backend->setClearColor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f));
	gfx_backend->clearBuffer();

	ImGui::SliderFloat("light power", &point_light_power, 100.0f, 2000.0f);

	gfx_backend->bindGPUProgram(gpu_program);

	gfx_backend->setGPUProgramMat4(gpu_program, "view", view);
	gfx_backend->setGPUProgramMat4(gpu_program, "projection", projection);

	gfx_backend->setGPUProgramVec3(gpu_program, "lightPos", point_light_pos);
	gfx_backend->setGPUProgramVec3(gpu_program, "viewPos", cameraPosition);
	gfx_backend->setGPUProgramVec3(gpu_program, "lightColor", glm::vec3(1, 1, 1));
	gfx_backend->setGPUProgramFloat(gpu_program, "lightPower", point_light_power);

	scene_cyclorama->draw();
	sphere->draw();
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
	gfx_backend->init("point light example", scrn_width, scrn_height);
	gfx_backend->on_Init(init);
	gfx_backend->on_Render(render);
	gfx_backend->on_Resize(resize);
	gfx_backend->start();

	return 0;
}