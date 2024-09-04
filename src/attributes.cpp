#include "attributes.h"

namespace gfx
{
	// default constructor
	Attributes::Attributes() { m_size = 0; }

	// vector init constructor
	Attributes::Attributes(std::vector<GPU_Attribute>& e)
	{
		m_attributes = e;
		calcSize();
	}

	// raw pointer init constructor
	Attributes::Attributes(GPU_Attribute* e, uint32_t count)
	{
		for (int i = 0; i < count; i++)
			m_attributes.push_back(e[i]);

		calcSize();
	}

	// copy constrcutor
	Attributes::Attributes(const Attributes& val)
	{
		m_attributes = val.m_attributes;
		m_size = val.m_size;
	}

	//
	void
	Attributes::append(GPU_Attribute v)
	{
		if (v.offset == 0)
			v.offset = m_size;

		m_attributes.push_back(v);
		m_size += v.size;
	}

	// size getter
	uint32_t
	Attributes::getSize() const
	{
		return m_size;
	}

	// count getter
	uint32_t
	Attributes::getElementCount() const
	{
		return m_attributes.size();
	}

	// equality check function
	bool
	Attributes::equals(const Attributes& val)
	{
		if (m_size != val.m_size)
			return false;

		if (val.m_attributes.size() != m_attributes.size())
			return false;

		for (int i = 0; i < m_attributes.size(); i++)
			if (!m_attributes[i].equals(val.m_attributes[i]))
			{
				return false;
			}

		return true;
	}

	// destrcutor
	Attributes::~Attributes() { m_attributes.clear(); }

	uint32_t
	Attributes::calcSize()
	{
		m_size = 0;
		for (auto attrib : m_attributes)
		{
			m_size += attrib.size;
		}
		return m_size;
	}
} // namespace gfx