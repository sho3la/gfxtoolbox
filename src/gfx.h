#pragma once
#include "Image.h"
#include "attributes.h"
#include "enums.h"
#include "gpu_attribute.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <iostream>
#include <string>

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
		start(void (*init)(), void (*input)(GLFWwindow* window), void (*render)());

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
		createGPUProgram(const char* vs, const char* fs);

		void
		bindGPUProgram(uint32_t gpu_program);

		void
		bindTexture1D(uint32_t texture1d);

		void
		bindTexture2D(uint32_t texture2d);

		void
		draw(GFX_Primitive type, uint32_t gpu_mesh_id, uint32_t vertices_count);

		void
		draw_indexed(GFX_Primitive type, uint32_t gpu_mesh_id, uint32_t indices_count);

	private:
		glm::vec4 m_clearcolor;
		GLFWwindow* window;
	};
} // namespace gfx