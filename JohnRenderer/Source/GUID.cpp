#include "pch.h"
#include "GUID.h"
#include <random>

static std::random_device s_RandomDevice;
static std::mt19937_64 eng(s_RandomDevice());
static std::uniform_int_distribution<uint64_t> s_UniformDistribution;


GUID::GUID()
	:m_GUID(s_UniformDistribution(eng))
{


}

GUID::GUID(uint64_t guid)
	:m_GUID(guid)
{

}

GUID::GUID(const GUID& other)
	:m_GUID(other.m_GUID)
{

}
