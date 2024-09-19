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

		bool
		hasData();

		int
		getWidth();

		int
		getHeight();

		int
		getDepth();

	private:
		int width, height, depth;
		std::vector<std::vector<std::vector<float>>> data;

		bool
		isValidPixel(int x, int y, int z);
	};
} // namespace gfx