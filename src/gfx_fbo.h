#pragma once

namespace gfx
{
	class Framebuffer
	{
	public:
		Framebuffer(int width, int height);
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

		void
		createFramebuffer();

		void
		deleteFramebuffer();
	};
} // namespace gfx