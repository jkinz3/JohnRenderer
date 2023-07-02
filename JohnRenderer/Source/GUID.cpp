#include "pch.h"
#include "GUID.h"
#include <random>

static std::random_device s_RandomDevice;
static std::mt19937_64 eng(s_RandomDevice());
static std::uniform_int_distribution<uint64_t> s_UniformDistribution;


JohnGUID::JohnGUID()
	:m_JohnGUID(s_UniformDistribution(eng))
{


}

JohnGUID::JohnGUID(uint64_t guid)
	:m_JohnGUID(guid)
{

}

JohnGUID::JohnGUID(const JohnGUID& other)
	:m_JohnGUID(other.m_JohnGUID)
{

}
