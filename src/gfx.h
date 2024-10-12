#pragma once
#include "Image.h"
#include "Image3D.h"
#include "attributes.h"
#include "enums.h"
#include "gpu_attribute.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <map>
#include <functional>

namespace gfx
{
	class GFX
	{
	public:
		GFX();

		~GFX();

		bool
		init(const char* window_title, int window_width, int window_height);

		void
		on_Init(std::function<void()> function);

		void
		on_Render(std::function<void()> function);

		void
		on_Resize(std::function<void(int width, int height)> function);

		void
		on_MouseMove(std::function<void(double x, double y)> function);

		void
		on_MouseScroll(std::function<void(double xoffset, double yoffset)> function);

		void
		on_MouseButton(std::function<void(int button, int action, int mods)> function);

		void
		start();

		glm::vec2
		getMouse_position();

		void
		setClearColor(glm::vec4 color);

		void
		clearBuffer();

		void
		updateViewport(uint32_t width, uint32_t height);

		uint32_t
		createVertexBuffer(void* data, uint32_t size, BUFFER_USAGE usage);

		uint32_t
		createIndexBuffer(void* data, uint32_t size, BUFFER_USAGE usage);

		uint32_t
		createGPUMesh(uint32_t vertex_buffer, const Attributes& attribs);

		uint32_t
		createGPUMesh(uint32_t vertex_buffer, uint32_t index_buffer, const Attributes& attribs);

		uint32_t
		createTexture1D(
			Image* img,
			Wrapping_Mode wrap_mode,
			Filtering_Mode minifying_mode,
			Filtering_Mode magnifying_mode);

		uint32_t
		createTexture2D(
			Image* img,
			Wrapping_Mode wrap_mode,
			Filtering_Mode minifying_mode,
			Filtering_Mode magnifying_mode,
			bool enable_mipmaps);

		uint32_t
		createTexture3D(
			Image3D* img,
			Wrapping_Mode wrap_mode,
			Filtering_Mode minifying_mode,
			Filtering_Mode magnifying_mode,
			bool enable_mipmaps);

		uint32_t
		createGPUProgram(const char* vs, const char* fs);

		uint32_t
		createGPUProgram(const char* vs, const char* gs, const char* fs);

		void
		bindGPUProgram(uint32_t gpu_program);

		void
		setGPUProgramVec2(uint32_t gpu_program, const std::string& name, const glm::vec2& val);

		void
		setGPUProgramVec3(uint32_t gpu_program, const std::string& name, const glm::vec3& val);

		void
		setGPUProgramMat4(uint32_t gpu_program, const std::string& name, const glm::mat4& mat);

		void
		setGPUProgramFloat(uint32_t gpu_program, const std::string& name, const float& val);

		void
		setGPUProgramInt(uint32_t gpu_program, const std::string& name, const int& val);

		void
		bindTexture1D(uint32_t texture1d);

		void
		bindTexture2D(uint32_t texture2d);

		void
		bindTexture3D(uint32_t texture3d);

		void
		draw(GFX_Primitive type, uint32_t gpu_mesh_id, uint32_t vertices_count);

		void
		draw_indexed(GFX_Primitive type, uint32_t gpu_mesh_id, uint32_t indices_count);

	private:
		glm::vec4 m_clearcolor;
		GLFWwindow* window;
		
		// gfx api callbacks
		std::function<void()> m_initCallback;
		std::function<void()> m_renderCallback;
		std::function<void(int width, int height)> m_resizeCallback;
		std::function<void(double xoffset, double yoffset)> m_scrollCallback;
		std::function<void(double x, double y)> m_mouseMoveCallback;
		std::function<void(int button, int action, int mods)> m_mousePressedCallback;
	};
} // namespace gfx