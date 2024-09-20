#pragma once

#include <vector>

namespace gfx
{
	class Image3D
	{
	public:
		Image3D(int width, int height, int depth);

		~Image3D();

		void
		setPixel(int x, int y, int z, float value);

		float
		getPixel(int x, int y, int z);

		std::vector<float>
		getData();

		void
		setData(std::vector<float>& data);

		bool
		hasData();

		int
		getWidth();

		int
		getHeight();

		int
		getDepth();

		std::vector<float>
		gradientsBuffer()
		{
			const int size = width * height * depth * 3;
			std::vector<float> buffer(size);

			int index = 0;
			for (int z = 0; z < depth; ++z)
			{
				for (int y = 0; y < height; ++y)
				{
					for (int x = 0; x < width; ++x)
					{
						if (z >= width - 1 || z >= height - 1)
							continue;

						if (y >= width - 1 || y >= depth - 1)
							continue;

						if (x >= depth - 1 || x >= height - 1)
							continue;

						// Calculate gradient for x
						float gradX = (x == 0 || x == width - 1) ? 0 : data[x - 1][y][z] - data[x + 1][y][z];

						// Calculate gradient for y
						float gradY = (y == 0 || y == height - 1) ? 0 : data[x][y - 1][z] - data[x][y + 1][z];

						// Calculate gradient for z
						float gradZ = (z == 0 || z == depth - 1) ? 0 : data[x][y][z + 1] - data[x][y][z - 1];

						// Store the gradients in the buffer
						buffer[index++] = gradX;
						buffer[index++] = gradY;
						buffer[index++] = gradZ;
					}
				}
			}

			return buffer; // Return the buffer containing gradients
		}

	private:
		int width, height, depth;
		std::vector<std::vector<std::vector<float>>> data;

		bool
		isValidPixel(int x, int y, int z);
	};
} // namespace gfx