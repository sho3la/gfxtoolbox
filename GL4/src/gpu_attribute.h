#pragma once

#include <string>

namespace gfx
{
	class GPU_Attribute
	{
	public:
		// type of the attribute
		enum Type
		{
			VEC2 = 0,
			VEC3,
			VEC4,
			FLOAT,
			INT,
			BOOL,
			MAT3,
			MAT4,
			NONE
		};

		// type of the vertex element
		Type type;

		// element start offset in bytes
		uint32_t offset;

		// element size in bytes
		uint32_t size;

		/// element component count
		uint32_t components;

		// semantic of the input layout
		std::string semantic;

		// init constructor
		GPU_Attribute(
			uint32_t _offset,
			uint32_t _size,
			uint32_t _components,
			Type _type = NONE,
			std::string _semantic = "");

		// size deduced type constructor
		GPU_Attribute(Type _type, std::string _semantic);
		
		// copy constructor
		GPU_Attribute(const GPU_Attribute& val);

		// default destructor
		~GPU_Attribute();

		// comparision function
		bool
		equals(const GPU_Attribute& val);
	};

} // namespace gfx