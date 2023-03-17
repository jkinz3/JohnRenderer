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

	float ClampAxis(float Angle)
	{
		Angle = std::fmod( Angle, 360.f );
		if ( Angle < 0.f )
		{
			Angle += 360.f;
		}
		return Angle;

	}

	float NormalizeAxis(float Angle)
	{
		Angle = ClampAxis( Angle );
		if(Angle > 180.f)
		{
			Angle -= 360.f;
		}
		return Angle;
	}

	DirectX::SimpleMath::Vector3 ConvertToEulerUEStyle( Quaternion InQuat )
	{
		float X = InQuat.x;
		float Y = InQuat.y;
		float Z = InQuat.z;
		float W = InQuat.w;

		float Pitch, Yaw, Roll;

		const float SingularityTest = Z * X - W * Y;
		const float YawY = 2.f * (W * Z + X * Y);
		const float YawX = (1.f - 2.f * ((Y *Y ) + ( Z *Z)));
		
		const float SINGULARITY_THRESHOLD = 0.4999995f;
		const float RAD_TO_DEG = (180.f / DirectX::XM_PI);
		
		if ( SingularityTest < -SINGULARITY_THRESHOLD )
		{
			Pitch = -90.f;
			Yaw = (std::atan2( YawY, YawX ) * RAD_TO_DEG);
			Roll = NormalizeAxis( -Yaw - (2.f * std::atan2( X, W ) * RAD_TO_DEG) );
		}
		else if ( SingularityTest > SINGULARITY_THRESHOLD )
		{
			Pitch = 90.f;
			Yaw = (std::atan2( YawY, YawX ) * RAD_TO_DEG);
			Roll = NormalizeAxis( Yaw - (2.f * std::atan2( X, W ) * RAD_TO_DEG) );
		}
		else
		{
			Pitch = (std::asin( 2.f * SingularityTest ) * RAD_TO_DEG);
			Yaw = (std::atan2( YawY, YawX ) * RAD_TO_DEG);
			Roll = (std::atan2( -2.f * (W * X + Y * Z), (1.f - 2.f * (( X*X ) + ( Y*Y ))) ) * RAD_TO_DEG);
		}

		Vector3 euler( Roll, Pitch, Yaw);

		return euler;
	}

}
