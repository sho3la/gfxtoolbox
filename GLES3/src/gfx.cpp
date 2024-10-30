#include "gfx.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

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

	// API
	GFX::GFX() : m_clearcolor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f)) {}

	GFX::~GFX()
	{
		// Cleanup
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		// glfw: terminate, clearing all previously allocated GLFW resources.
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	bool
	GFX::init(const char* window_title, int window_width, int window_height)
	{
		// glfw: initialize and configure
		glfwInit();
		glfwWindowHint(GLFW_SAMPLES, 8);
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

		auto res = glewInit();
		if (res != GLEW_OK)
		{
			auto mssage = glewGetErrorString(res);
			std::cout << "Failed to initialize GLEW " << mssage << std::endl;

			return false;
		}

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 130");

		// configure global opengl state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);

		return true;
	}

	void
	GFX::on_Init(std::function<void()> function)
	{
		m_initCallback = function;
	}

	void
	GFX::on_Render(std::function<void()> function)
	{
		m_renderCallback = function;
	}

	void
	GFX::on_Resize(std::function<void(int width, int height)> function)
	{
		m_resizeCallback = function;
		glfwSetWindowUserPointer(window, this);

		auto inner_call = [](GLFWwindow* window, int width, int height) {
			// Capture 'this' to access the member variable
			auto gfx = static_cast<GFX*>(glfwGetWindowUserPointer(window));
			if (gfx)
			{
				glViewport(0, 0, width, height);

				if (gfx->m_resizeCallback)
					gfx->m_resizeCallback(width, height);
			}
		};

		glfwSetFramebufferSizeCallback(window, inner_call);
	}

	void
	GFX::on_MouseMove(std::function<void(double x, double y)> function)
	{
		m_mouseMoveCallback = function;
		glfwSetWindowUserPointer(window, this);

		auto inner_call = [](GLFWwindow* window, double x, double y) {
			// Capture 'this' to access the member variable
			auto gfx = static_cast<GFX*>(glfwGetWindowUserPointer(window));
			if (gfx)
			{
				if (gfx->m_mouseMoveCallback)
					gfx->m_mouseMoveCallback(x, y);

				ImGui_ImplGlfw_CursorPosCallback(window, x, y);
			}
		};

		glfwSetCursorPosCallback(window, inner_call);
	}

	void
	GFX::on_MouseScroll(std::function<void(double xoffset, double yoffset)> function)
	{
		m_scrollCallback = function;
		glfwSetWindowUserPointer(window, this);

		auto inner_call = [](GLFWwindow* window, double xoffset, double yoffset) {
			// Capture 'this' to access the member variable
			auto gfx = static_cast<GFX*>(glfwGetWindowUserPointer(window));
			if (gfx)
			{
				ImGuiIO& io = ImGui::GetIO();
				if (io.WantCaptureMouse)
					ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
				else if (gfx->m_scrollCallback)
					gfx->m_scrollCallback(xoffset, yoffset);
			}
		};

		glfwSetScrollCallback(window, inner_call);
	}

	void
	GFX::on_MouseButton(std::function<void(int button, int action, int mods)> function)
	{
		m_mousePressedCallback = function;
		glfwSetWindowUserPointer(window, this);

		auto inner_call = [](GLFWwindow* window, int button, int action, int mods) {
			// Capture 'this' to access the member variable
			auto gfx = static_cast<GFX*>(glfwGetWindowUserPointer(window));
			if (gfx)
			{
				ImGuiIO& io = ImGui::GetIO();
				if (io.WantCaptureMouse)
					ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
				else if (gfx->m_mousePressedCallback)
					gfx->m_mousePressedCallback(button, action, mods);
			}
		};

		glfwSetMouseButtonCallback(window, inner_call);
	}

	void
	GFX::start()
	{
		if (m_initCallback)
			m_initCallback();

		// call default resizing
		on_Resize(m_resizeCallback);

		// render loop
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();

			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			if (m_renderCallback)
				m_renderCallback();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window);
		}
	}

	glm::vec2
	GFX::getMouse_position()
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		return glm::vec2(xpos, ypos);
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
	GFX::enableSetting(GFX_Settings settings)
	{
		switch (settings)
		{
		case gfx::DEPTH_TEST:
			glEnable(GL_DEPTH_TEST);
			break;
		case gfx::BLENDING:
			glEnable(GL_BLEND);
			break;
		case gfx::CULLING:
			glEnable(GL_CULL_FACE);
			break;
		default:
			break;
		}
	}

	void
	GFX::disableSetting(GFX_Settings settings)
	{
		switch (settings)
		{
		case gfx::DEPTH_TEST:
			glDisable(GL_DEPTH_TEST);
			break;
		case gfx::BLENDING:
			glDisable(GL_BLEND);
			break;
		case gfx::CULLING:
			glDisable(GL_CULL_FACE);
			break;
		default:
			break;
		}
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
	GFX::createTexture3D(
		Image3D* img,
		Wrapping_Mode wrap_mode,
		Filtering_Mode minifying_mode,
		Filtering_Mode magnifying_mode,
		bool enable_mipmaps)
	{
		GLuint id = -1;

		if (!img->hasData())
		{
			std::cout << "Empty image check image file" << std::endl;
			return id;
		}

		glGenTextures(1, &id);

		if (id == -1)
		{
			std::cout << "Cannot generate Texture3D" << std::endl;
			return id;
		}

		glBindTexture(GL_TEXTURE_3D, id);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage3D(
			GL_TEXTURE_3D,
			0,
			GL_R32F,
			img->getWidth(),
			img->getHeight(),
			img->getDepth(),
			0,
			GL_R32F,
			GL_FLOAT,
			img->getData().data());

		if (enable_mipmaps)
			glGenerateMipmap(GL_TEXTURE_3D);

		auto res = _wrapping_mode(wrap_mode);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, res);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, res);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, res);

		auto minifying = _filtering_mode(minifying_mode);
		auto magnifying = _filtering_mode(magnifying_mode);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, minifying);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, magnifying);

		glBindTexture(GL_TEXTURE_3D, 0);

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

	uint32_t
	GFX::createGPUProgram(const char* vs, const char* gs, const char* fs)
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

		// geometry shader
		unsigned int geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader, 1, &gs, NULL);
		glCompileShader(geometryShader);

		// check for shader compile errors
		glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
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
		glAttachShader(shaderProgram, geometryShader);
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
		glDeleteShader(geometryShader);
		glDeleteShader(fragmentShader);

		return shaderProgram;
	}

	void
	GFX::bindGPUProgram(uint32_t gpu_program)
	{
		glUseProgram(gpu_program);
	}

	void
	GFX::setGPUProgramVec2(uint32_t gpu_program, const std::string& name, const glm::vec2& val)
	{
		glUniform2fv(glGetUniformLocation(gpu_program, name.c_str()), 1, glm::value_ptr(val));
	}

	void
	GFX::setGPUProgramVec3(uint32_t gpu_program, const std::string& name, const glm::vec3& val)
	{
		glUniform3fv(glGetUniformLocation(gpu_program, name.c_str()), 1, glm::value_ptr(val));
	}

	void
	GFX::setGPUProgramMat4(uint32_t gpu_program, const std::string& name, const glm::mat4& mat)
	{
		glUniformMatrix4fv(glGetUniformLocation(gpu_program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	void
	GFX::setGPUProgramFloat(uint32_t gpu_program, const std::string& name, const float& val)
	{
		glUniform1f(glGetUniformLocation(gpu_program, name.c_str()), val);
	}

	void
	GFX::setGPUProgramInt(uint32_t gpu_program, const std::string& name, const int& val)
	{
		glUniform1i(glGetUniformLocation(gpu_program, name.c_str()), val);
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
	GFX::bindTexture3D(uint32_t texture3d)
	{
		glBindTexture(GL_TEXTURE_3D, texture3d);
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
		else if (type == TRIANGLES_STRIP)
			glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices_count);
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