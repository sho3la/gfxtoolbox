#pragma once

#include <memory>
#include <stb_image.h>

namespace gfx
{

	class Image
	{
	public:
		Image();

		Image(const char* file_name);

		Image(unsigned char* data, int width, int ncomponents);

		~Image();

		unsigned char*
		getData();

		uint32_t
		getWidth();

		uint32_t
		getHeight();

		uint32_t
		get_NCompnents();

		// pointer to image pixels
		unsigned char* m_data;
		int m_width, m_height, m_ncomponents;
	};

} // namespace gfx