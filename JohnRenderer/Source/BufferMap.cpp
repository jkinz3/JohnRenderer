#include "pch.h"
#include "BufferMap.h"

size_t BufferDescriptor::GetHash()
{
	if ( !bHashCalculated )
	{
		std::hash<BufferDescriptor> bufferHash;
		m_Hash = bufferHash(*this);
		bHashCalculated = true;
	}
	return m_Hash;
}
