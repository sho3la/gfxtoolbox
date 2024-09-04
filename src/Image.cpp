#include "Image.h"

namespace gfx
{
	Image::Image()
	{
		m_width = 0;
		m_height = 0;
		m_ncomponents = 0;
		m_data = nullptr;
	}

	Image::Image(const char* file_name) : m_width(0), m_height(0), m_ncomponents(0), m_data(nullptr)
	{
		m_data = stbi_load(file_name, &m_width, &m_height, &m_ncomponents, 0);
	}

	Image::Image(unsigned char* data, int width, int ncomponents) : m_height(0)
	{
		m_width = width;
		m_ncomponents = ncomponents;

		size_t totoal_size = m_width * m_ncomponents;
		m_data = new unsigned char[totoal_size];
		memcpy(m_data, data, totoal_size);
	}

	Image::~Image()
	{
		if (m_data)
			stbi_image_free(m_data);
	}

	unsigned char*
	Image::getData()
	{
		return m_data;
	}

	uint32_t
	Image::getWidth()
	{
		return m_width;
	}

	uint32_t
	Image::getHeight()
	{
		return m_height;
	}

	uint32_t
	Image::get_NCompnents()
	{
		return m_ncomponents;
	}
} // namespace gfx