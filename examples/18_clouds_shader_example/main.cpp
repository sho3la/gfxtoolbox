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

OrbitalCamera camera(68.0f, -0.1f, 25.0f);

auto gfx_backend = std::make_shared<gfx::GFX>();

int scrn_width = 800;
int scrn_height = 600;

uint32_t vertex_buffer_id, gpu_mesh_id, gpu_program;
uint32_t grid_vertex_buffer_id, grid_gpu_mesh_id, grid_gpu_program;

// create transformations
glm::mat4 model = glm::mat4(1.0f);
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

const char* grid_vertexShader = R"(
		#version 450 core

		layout (location = 0) in vec3 v_pos;
		layout (location = 1) in vec3 v_col;

		uniform mat4 mvp;
		smooth out vec3 f_col;

		void main() {
			gl_Position = mvp * vec4(v_pos, 1.0f);
			f_col = v_col;
		})";

const char* grid_fragmentShader =R"(
		#version 450 core

		layout (location = 0) out vec4 out_col;

		smooth in vec3 f_col;

		void main() {
			out_col = vec4(f_col, 1.0f);
		})";

const char* vertexShader = R"(
		#version 450 core

		layout (location = 0) in vec3 Position;

		out vec2 v;

		void main()
		{
			v = Position.xy;
			gl_Position = vec4(Position, 1.0);
		})";

// refrence shader : https://www.shadertoy.com/view/4dl3z7
const char* fragmentShader =R"(
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
			stars = scol * (.3+fbm(rd)) * 3;
			col += stars;

			// Clouds
			vec2 shift = vec2( 2000.0, 2080.0 );
			vec4 sum = vec4(0,0,0,0); 
			for (int q=1000; q<1060; q++) // 120 layers
			{
				if (sum.w>0.999)
				 break;
				float c = (float(q-1000)*10.0+350.0-cameraPos.y) / rd.y; // cloud height
				vec3 cpos = cameraPos + c*rd + vec3(831.0+shift.x, 321.0+float(q-1000)*.15-shift.x*0.2, 1330.0+shift.y); // cloud position
			
				float alpha = smoothstep(0.5, 1.0, fbm( cpos*0.0015 ))*.9; // fractal cloud density
				vec3 localcolor = mix(vec3( 1.1, 1.05, 1.0 ), 0.7*vec3( 0.4,0.4,0.3 ), alpha); // density color white->gray
				alpha = (1.0-sum.w)*alpha; // alpha/density saturation (the more a cloud layer's density, the more the higher layers will be hidden)
				sum += vec4(localcolor*alpha, alpha); // sum up weightened color
			}
			
				float alpha = smoothstep(0.7, 1.0, sum.w);
				float dotval = max(dot(rd,moondir),0.);
				sum.rgb /= sum.w+0.0001;
				sum.rgb -= 0.6*vec3(0.8, 0.75, 0.7) * pow(dotval,10.0)*alpha;
				sum.rgb += 0.2*vec3(1.2, 1.2, 1.2) * pow(dotval,5.0)*(1.0-alpha);

				col = mix( col, sum.rgb , 1.0*sum.w*pow(dotval,3.0)*(1.0-pow(t,10.)) );

			return col;
		}

		void main()
		{
			vec3 dir = vec3( normalize( inv_view * vec4(v.xy*vec2(resolution.x/resolution.y,1),-1.5,0.0) ) );
			FragColor = vec4(skyColor(dir),1.0);
		})";

// clang-format on

inline static void
_init_grid()
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

	grid_vertex_buffer_id =
		gfx_backend->createVertexBuffer(gpu_arr.data(), sizeof(gpu_arr[0]) * gpu_arr.size(), gfx::BUFFER_USAGE::STATIC);

	gfx::Attributes attributes;
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "POSITION"));
	attributes.append(gfx::GPU_Attribute(gfx::GPU_Attribute::VEC3, "COLOR"));

	grid_gpu_mesh_id = gfx_backend->createGPUMesh(grid_vertex_buffer_id, attributes);

	// build and compile our shader program
	grid_gpu_program = gfx_backend->createGPUProgram(grid_vertexShader, grid_fragmentShader);
}

inline static void
_draw_grid()
{
	gfx_backend->bindGPUProgram(grid_gpu_program);

	view = camera.getViewMatrix();

	// send to gpu
	auto mvp = projection * view * model;
	gfx_backend->setGPUProgramMat4(grid_gpu_program, "mvp", mvp);

	glLineWidth(1.0f);
	gfx_backend->draw(gfx::GFX_Primitive::LINES, grid_gpu_mesh_id, verts.size());
}

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

	_init_grid();
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
	gfx_backend->setGPUProgramVec3(gpu_program, "cameraPos", camera.getPosition());

	glDisable(GL_DEPTH_TEST);
	gfx_backend->draw(gfx::GFX_Primitive::TRIANGLES_STRIP, gpu_mesh_id, 4);
	glEnable(GL_DEPTH_TEST);

	//_draw_grid();
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
	gfx_backend->init("clouds shader example", scrn_width, scrn_height);
	gfx_backend->on_Init(init);
	gfx_backend->on_Render(render);
	gfx_backend->on_Resize(resize);
	gfx_backend->on_MouseScroll(mouse_scroll);
	gfx_backend->on_MouseMove(mouse_move);
	gfx_backend->on_MouseButton(mouse_button);
	gfx_backend->start();

	return 0;
}