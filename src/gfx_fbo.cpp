#include "gfx_fbo.h"

#include <GL/glew.h>

#include <iostream>

namespace gfx
{
	Framebuffer::Framebuffer(int width, int height, FrameBuffer_Mode mode) : width(width), height(height), mode(mode)
	{
		switch (mode)
		{
		case gfx::RenderBuffer:
			createRenderBuffer();
			break;
		case gfx::DepthCubeMap:
			createDepthCubeMapBuffer();
			break;
		default:
			break;
		}
	}

	Framebuffer::~Framebuffer()
	{
		switch (mode)
		{
		case gfx::RenderBuffer:
			deleteRenderBuffer();
			break;
		case gfx::DepthCubeMap:
			deleteDepthCubeMapBuffer();
			break;
		default:
			break;
		}
	}

	void
	Framebuffer::createRenderBuffer()
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// create a color attachment texture
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

		// Create renderbuffer for depth and stencil
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);

		// use a single renderbuffer object for both a depth AND stencil buffer.
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

		// attach frame buffer
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Framebuffer is not complete!" << std::endl;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void
	Framebuffer::deleteRenderBuffer()
	{
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &texture);
		glDeleteRenderbuffers(1, &rbo);
	}

	void
	Framebuffer::createDepthCubeMapBuffer()
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// create depth cubemap texture
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_DEPTH_COMPONENT,
				width,
				height,
				0,
				GL_DEPTH_COMPONENT,
				GL_FLOAT,
				NULL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void
	Framebuffer::deleteDepthCubeMapBuffer()
	{
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &texture);
	}

	void
	Framebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}

	void
	Framebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void
	Framebuffer::Resize(int newWidth, int newHeight)
	{
		if (newWidth != width || newHeight != height)
		{
			deleteRenderBuffer();
			width = newWidth;
			height = newHeight;
			createRenderBuffer();
		}
	}

	unsigned int
	Framebuffer::GetTexture() const
	{
		return texture;
	}
} // namespace gfx