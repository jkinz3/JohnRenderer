#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class BufferDescriptor
{
public:
	BufferDescriptor();

	size_t GetHash();

	std::string GetBufferName() const { return BufferName; }
	
private:

	bool bHashCalculated = false;
	size_t m_Hash;
	unsigned int m_AccessorIndex;
	std::string BufferName;


};

class Buffer
{
public:
	static std::shared_ptr<Buffer> Create(BufferDescriptor)
	{

	}
};

template<>
struct std::hash<BufferDescriptor>
{
	size_t operator()(const BufferDescriptor& descriptor) const
	{
		size_t res = 0;
		std::hash<const char*> myHash;
		res ^= myHash(descriptor.GetBufferName().c_str());
		return res;
	}
};

class BufferMap
{
};

