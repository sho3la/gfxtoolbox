#pragma once

#include "enums.h"

namespace gfx
{
	class Framebuffer
	{
	public:
		Framebuffer(int width, int height, FrameBuffer_Mode mode = RenderBuffer);
		~Framebuffer();

		void
		Bind();

		void
		Unbind();

		void
		Resize(int newWidth, int newHeight);

		unsigned int
		GetTexture() const;

	private:
		unsigned int fbo;
		unsigned int texture;
		unsigned int rbo; // Renderbuffer object for depth/stencil
		int width;
		int height;
		FrameBuffer_Mode mode;

		void
		createRenderBuffer();

		void
		deleteRenderBuffer();

		void
		createDepthCubeMapBuffer();

		void
		deleteDepthCubeMapBuffer();
	};
} // namespace gfx