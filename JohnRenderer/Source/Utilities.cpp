#include "pch.h"
#include "Utilities.h"
namespace John
{
	
	std::string John::ConvertToUTF8(const std::wstring& wstr)
	{
		const int bufferSize = WideCharToMultiByte (CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		std::string buffer(bufferSize, 0);
		WideCharToMultiByte (CP_UTF8, 0, wstr.c_str (), -1, &buffer[0], bufferSize, NULL, NULL);
		return std::string(&buffer[0]);
	}
	
	std::wstring John::ConvertToUTF16(const std::string& str)
	{
		const int bufferSize = MultiByteToWideChar (CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
		std::wstring buffer(bufferSize, 0);
		MultiByteToWideChar (CP_UTF8, 0, str.c_str (), -1, &buffer[0], bufferSize);
		return std::wstring(&buffer[0]);
	}
}
