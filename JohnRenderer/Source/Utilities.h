#pragma once

namespace John
{
	std::string ConvertToUTF8(const std::wstring& wstr);
	std::wstring ConvertToUTF16(const std::string& str);
}