#include "gfx.h"

#include <iostream>

namespace gfx
{
	inline static GLenum
	_wrapping_mode(Wrapping_Mode wrap_mode)
	{
		GLenum res = 0;
		switch (wrap_mode)
		{
		case REPEAT:
			res = GL_REPEAT;
			break;

		case CLAMP_TO_EDGE:
			res = GL_CLAMP_TO_EDGE;
			break;

		case CLAMP_TO_BORDER:
			res = GL_CLAMP_TO_BORDER;
			break;

		case MIRRORED_REPEAT:
			res = GL_MIRRORED_REPEAT;
			break;
		}

		return res;
	}

	inline static GLint
	_filtering_mode(Filtering_Mode mode)
	{
		GLint res = 0;

		switch (mode)
		{
		case NEAREST:
			res = GL_NEAREST;
			break;

		case LINEAR:
			res = GL_LINEAR;
			break;

		case NEAREST_MIPMAP_NEAREST:
			res = GL_NEAREST_MIPMAP_NEAREST;
			break;

		case LINEAR_MIPMAP_NEAREST:
			res = GL_LINEAR_MIPMAP_NEAREST;
			break;

		case NEAREST_MIPMAP_LINEAR:
			res = GL_NEAREST_MIPMAP_LINEAR;
			break;

		case LINEAR_MIPMAP_LINEAR:
			res = GL_LINEAR_MIPMAP_LINEAR;
			break;
		}

		return res;
	}

	// glfw: whenever the window size changed (by OS or user resize) this callback function executes
	void
	framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		// make sure the viewport matches the new window dimensions; note that width and
		// height will be significantly larger than specified on retina displays.
		glViewport(0, 0, width, height);
	}

	// API
	GFX::GFX() : m_clearcolor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f)) {}

	GFX::~GFX()
	{
		// glfw: terminate, clearing all previously allocated GLFW resources.
		glfwTerminate();
	}

	bool
	GFX::init(const char* window_title, int window_width, int window_height)
	{
		// glfw: initialize and configure
		glfwInit();
		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// glfw window creation
		window = glfwCreateWindow(window_width, window_height, window_title, NULL, NULL);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return false;
		}
		glfwMakeContextCurrent(window);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

		auto res = glewInit();
		if (res != GLEW_OK)
		{
			auto mssage = glewGetErrorString(res);
			std::cout << "Failed to initialize GLEW " << mssage << std::endl;

			return false;
		}
		return true;
	}

	void
	GFX::start(void (*init)(), void (*input)(GLFWwindow* window), void (*render)())
	{
		init();

		// render loop
		while (!glfwWindowShouldClose(window))
		{

			input(window);

			render();

			// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
			// -------------------------------------------------------------------------------
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}

	void
	GFX::setClearColor(glm::vec4 color)
	{
		m_clearcolor = color;
		glClearColor(m_clearcolor.r, m_clearcolor.g, m_clearcolor.b, m_clearcolor.a);
	}

	void
	GFX::clearBuffer()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void
	GFX::updateViewport(uint32_t width, uint32_t height)
	{
		glViewport(0, 0, width, height);
	}

	uint32_t
	GFX::createVertexBuffer(void* data, uint32_t size, BUFFER_USAGE usage)
	{
		GLuint id = -1;
		glGenBuffers(1, &id);

		if (id == -1)
		{
			std::cout << "Cannot generate vertex buffer" << std::endl;
			return id;
		}

		glBindBuffer(GL_ARRAY_BUFFER, id);

		switch (usage)
		{

			// static
		case BUFFER_USAGE::STATIC:
			glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
			break;

			// dynamic
		case BUFFER_USAGE::DYNAMIC:
			glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
			break;

		default:
			return id;
			break;
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return id;
	}

	uint32_t
	GFX::createIndexBuffer(void* data, uint32_t size, BUFFER_USAGE usage)
	{
		GLuint id = -1;
		glGenBuffers(1, &id);

		if (id == -1)
		{
			std::cout << "Cannot generate index buffer" << std::endl;
			return id;
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);

		switch (usage)
		{

			// static
		case BUFFER_USAGE::STATIC:
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
			break;

			// dynamic
		case BUFFER_USAGE::DYNAMIC:
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
			break;

		default:
			return id;
			break;
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return id;
	}

	uint32_t
	GFX::createGPUMesh(uint32_t vertex_buffer, const Attributes& attribs)
	{
		GLuint id = -1;
		glGenVertexArrays(1, &id);

		if (id == -1)
		{
			std::cout << "Cannot generate gpu mesh" << std::endl;
			return id;
		}

		glBindVertexArray(id);

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		for (int i = 0; i < attribs.getElementCount(); i++)
		{
			auto offset_val = static_cast<uintptr_t>(attribs.m_attributes[i].offset);

			glEnableVertexAttribArray(i);
			glVertexAttribPointer(
				i,
				attribs.m_attributes[i].components,
				GL_FLOAT,
				GL_FALSE,
				attribs.getSize(),
				reinterpret_cast<void*>(offset_val));
		}

		glBindVertexArray(0);

		return id;
	}

	uint32_t
	GFX::createGPUMesh(uint32_t vertex_buffer, uint32_t index_buffer, const Attributes& attribs)
	{
		GLuint id = -1;
		glGenVertexArrays(1, &id);

		if (id == -1)
		{
			std::cout << "Cannot generate gpu mesh" << std::endl;
			return id;
		}

		glBindVertexArray(id);

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

		for (int i = 0; i < attribs.getElementCount(); i++)
		{
			auto offset_val = static_cast<uintptr_t>(attribs.m_attributes[i].offset);

			glEnableVertexAttribArray(i);
			glVertexAttribPointer(
				i,
				attribs.m_attributes[i].components,
				GL_FLOAT,
				GL_FALSE,
				attribs.getSize(),
				reinterpret_cast<void*>(offset_val));
		}

		glBindVertexArray(0);

		return id;
	}

	uint32_t
	GFX::createTexture1D(
		Image* img,
		Wrapping_Mode wrap_mode,
		Filtering_Mode minifying_mode,
		Filtering_Mode magnifying_mode)
	{
		GLuint id = -1;

		if (!img->getData())
		{
			std::cout << "Empty image check image file" << std::endl;
			return id;
		}

		glGenTextures(1, &id);

		if (id == -1)
		{
			std::cout << "Cannot generate Texture1D" << std::endl;
			return id;
		}

		glBindTexture(GL_TEXTURE_1D, id);

		if (img->get_NCompnents() == 3)
			glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, img->getWidth(), 0, GL_RGB, GL_UNSIGNED_BYTE, img->getData());
		else if (img->get_NCompnents() == 4)
			glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, img->getWidth(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img->getData());

		auto res = _wrapping_mode(wrap_mode);

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, res);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, res);

		auto minifying = _filtering_mode(minifying_mode);
		auto magnifying = _filtering_mode(magnifying_mode);

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, minifying);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, magnifying);

		glBindTexture(GL_TEXTURE_1D, 0);

		return id;
	}

	uint32_t
	GFX::createTexture2D(
		Image* img,
		Wrapping_Mode wrap_mode,
		Filtering_Mode minifying_mode,
		Filtering_Mode magnifying_mode,
		bool enable_mipmaps)
	{
		GLuint id = -1;

		if (!img->getData())
		{
			std::cout << "Empty image check image file" << std::endl;
			return id;
		}

		glGenTextures(1, &id);

		if (id == -1)
		{
			std::cout << "Cannot generate Texture2D" << std::endl;
			return id;
		}

		glBindTexture(GL_TEXTURE_2D, id);

		if (img->get_NCompnents() == 3)
		{
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGB,
				img->getWidth(),
				img->getHeight(),
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				img->getData());
		}
		else if (img->get_NCompnents() == 4)
		{
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RGBA,
				img->getWidth(),
				img->getHeight(),
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				img->getData());
		}

		if (enable_mipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);

		auto res = _wrapping_mode(wrap_mode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, res);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, res);

		auto minifying = _filtering_mode(minifying_mode);
		auto magnifying = _filtering_mode(magnifying_mode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minifying);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnifying);

		glBindTexture(GL_TEXTURE_2D, 0);

		return id;
	}

	uint32_t
	GFX::createGPUProgram(const char* vs, const char* fs)
	{
		// vertex shader
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vs, NULL);
		glCompileShader(vertexShader);

		// check for shader compile errors
		int success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// fragment shader
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fs, NULL);
		glCompileShader(fragmentShader);
		// check for shader compile errors
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// link shaders
		uint32_t shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		// check for linking errors
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		return shaderProgram;
	}

	void
	GFX::bindGPUProgram(uint32_t gpu_program)
	{
		glUseProgram(gpu_program);
	}

	void
	GFX::bindTexture1D(uint32_t texture1d)
	{
		glBindTexture(GL_TEXTURE_1D, texture1d);
	}

	void
	GFX::bindTexture2D(uint32_t texture2d)
	{
		glBindTexture(GL_TEXTURE_2D, texture2d);
	}

	void
	GFX::draw(GFX_Primitive type, uint32_t gpu_mesh_id, uint32_t vertices_count)
	{

		glBindVertexArray(gpu_mesh_id);

		if (type == POINTS)
			glDrawArrays(GL_POINTS, 0, vertices_count);
		else if (type == LINES)
			glDrawArrays(GL_LINES, 0, vertices_count);
		else if (type == LINE_STRIP)
			glDrawArrays(GL_LINE_STRIP, 0, vertices_count);
		else if (type == TRIANGLES)
			glDrawArrays(GL_TRIANGLES, 0, vertices_count);
	}

	void
	GFX::draw_indexed(GFX_Primitive type, uint32_t gpu_mesh_id, uint32_t indices_count)
	{
		glBindVertexArray(gpu_mesh_id);

		if (type == POINTS)
			glDrawElements(GL_POINTS, indices_count, GL_UNSIGNED_INT, (void*)0);
		else if (type == LINES)
			glDrawElements(GL_LINES, indices_count, GL_UNSIGNED_INT, (void*)0);
		else if (type == LINE_STRIP)
			glDrawElements(GL_LINE_STRIP, indices_count, GL_UNSIGNED_INT, (void*)0);
		else if (type == TRIANGLES)
			glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, (void*)0);
	}

} // namespace gfx