#pragma once
using namespace DirectX::SimpleMath;
namespace John
{
	std::string ConvertToUTF8(const std::wstring& wstr);
	std::wstring ConvertToUTF16(const std::string& str);

	Vector3 ConvertToEulerUEStyle( Quaternion InQuat );
}