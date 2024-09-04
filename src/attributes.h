#pragma once

#include "gpu_attribute.h"

#include <vector>

namespace gfx
{
	class Attributes
	{
	public:
		// default constructor
		Attributes();

		// vector init constructor
		Attributes(std::vector<GPU_Attribute>& e);

		// raw pointer init constructor
		Attributes(GPU_Attribute* e, uint32_t count);

		// copy constrcutor
		Attributes(const Attributes& val);

		//
		void
		append(GPU_Attribute v);

		// size getter
		uint32_t
		getSize() const;

		// count getter
		uint32_t
		getElementCount() const;

		// equality check function
		bool
		equals(const Attributes& val);

		// destrcutor
		~Attributes();

		// elements vector
		std::vector<GPU_Attribute> m_attributes;

	protected:
		// size of full data
		uint32_t m_size;

		// calculates the size of the memory layout
		uint32_t
		calcSize();
	};
} // namespace gfx