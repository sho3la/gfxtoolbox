#include "gpu_attribute.h"

#include <glm/glm.hpp>

namespace gfx
{
	GPU_Attribute::GPU_Attribute(
		uint32_t _offset,
		uint32_t _size,
		uint32_t _components,
		Type _type,
		std::string _semantic)
	{
		offset = _offset;
		size = _size;
		components = _components;
		type = _type;
		semantic = _semantic;
	}

	GPU_Attribute::GPU_Attribute(Type _type, std::string _semantic)
	{
		type = _type;
		offset = 0;
		semantic = _semantic;
		switch (type)
		{
		case GPU_Attribute::VEC2:
			size = sizeof(glm::vec2);
			components = 2;
			break;
		case GPU_Attribute::VEC3:
			size = sizeof(glm::vec3);
			components = 3;
			break;
		case GPU_Attribute::VEC4:
			size = sizeof(glm::vec4);
			components = 4;
			break;
		case GPU_Attribute::FLOAT:
			size = sizeof(float);
			components = 1;
			break;
		case GPU_Attribute::INT:
			size = sizeof(int);
			components = 1;
			break;
		case GPU_Attribute::BOOL:
			size = sizeof(bool);
			components = 1;
			break;
		case GPU_Attribute::MAT3:
			size = sizeof(float) * 9;
			components = 1;
			break;
		case GPU_Attribute::MAT4:
			size = sizeof(float) * 16;
			components = 1;
			break;
		case GPU_Attribute::NONE:
			size = 0;
			components = 0;
			break;
		default:
			break;
		}
	}

	GPU_Attribute::GPU_Attribute(const GPU_Attribute& val)
	{
		offset = val.offset;
		size = val.size;
		components = val.components;
		type = val.type;
		semantic = val.semantic;
	}

	GPU_Attribute::~GPU_Attribute() {}

	bool
	GPU_Attribute::equals(const GPU_Attribute& val)
	{
		bool res = false;

		if (offset == val.offset && size == val.size && components == val.components && type == val.type &&
			semantic == val.semantic)
		{
			res = true;
		}

		return res;
	}
} // namespace gfx