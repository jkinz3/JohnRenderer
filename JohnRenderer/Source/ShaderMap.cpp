#include "pch.h"
#include "ShaderMap.h"

size_t ShaderDescriptor::GetHash()
{
	if(bHashCalculated)
	{
		std::hash<ShaderDescriptor> shaderHash;
		m_Hash = shaderHash(*this);
	}

	return m_Hash;
}
