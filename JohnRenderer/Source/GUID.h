#pragma once
#include <xhash>
class JohnGUID
{
public:
	JohnGUID();
	JohnGUID(uint64_t guid);
	JohnGUID(const JohnGUID& other);

	operator uint64_t() { return m_JohnGUID; }
	operator const uint64_t () const { return m_JohnGUID; }

private:
	uint64_t m_JohnGUID;
};

namespace std
{
	template<>
	struct hash<JohnGUID>
	{
		std::size_t operator()(const JohnGUID& guid) const
		{
			return guid;
		}
	};

}