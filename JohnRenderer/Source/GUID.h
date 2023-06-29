#pragma once
#include <xhash>
class GUID
{
public:
	GUID();
	GUID(uint64_t guid);
	GUID(const GUID& other);

	operator uint64_t() { return m_GUID; }
	operator const uint64_t () const { return m_GUID; }

private:
	uint64_t m_GUID;
};

namespace std
{
	template<>
	struct hash<GUID>
	{
		std::size_t operator()(const GUID& guid) const
		{
			return guid;
		}
	};

}