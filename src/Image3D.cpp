#include "Image3D.h"

#include <iostream>

namespace gfx
{
	Image3D::Image3D(int width, int height, int depth) : width(width), height(height), depth(depth)
	{
		// Initialize the 3D vector with zero values
		data.resize(depth, std::vector<std::vector<float>>(height, std::vector<float>(width, 0)));
	}

	Image3D::~Image3D() {}

	void
	Image3D::setPixel(int x, int y, int z, float value)
	{
		if (isValidPixel(x, y, z))
		{
			data[z][y][x] = value;
		}
		else
		{
			std::cout << "Pixel coordinates out of bounds!" << std::endl;
		}
	}

	float
	Image3D::getPixel(int x, int y, int z)
	{
		if (isValidPixel(x, y, z))
		{
			return data[z][y][x];
		}
		else
		{
			std::cout << "Pixel coordinates out of bounds!" << std::endl;
			return -1;
		}
	}

	std::vector<float>
	Image3D::getData()
	{
		std::vector<float> res;

		for (int z = 0; z < depth; ++z)
		{
			for (int y = 0; y < height; ++y)
			{
				for (int x = 0; x < width; ++x)
				{
					res.push_back(data[z][y][x]);
				}
			}
		}

		return res;
	}

	bool
	Image3D::hasData()
	{
		if (width > 0 && height > 0 && depth > 0)
			return true;

		return false;
	}

	int
	Image3D::getWidth()
	{
		return width;
	}

	int
	Image3D::getHeight()
	{
		return height;
	}

	int
	Image3D::getDepth()
	{
		return depth;
	}

	bool
	Image3D::isValidPixel(int x, int y, int z)
	{
		return (x >= 0 && x < width) && (y >= 0 && y < height) && (z >= 0 && z < depth);
	}
} // namespace gfx