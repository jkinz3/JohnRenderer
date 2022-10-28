#pragma once
#include "pch.h"

using Microsoft::WRL::ComPtr;

using namespace DirectX::SimpleMath;
using namespace DirectX;


namespace John
{
	struct ShaderProgram
	{
		ComPtr<ID3D11VertexShader> VertexShader;
		ComPtr<ID3D11PixelShader> PixelShader;
		ComPtr<ID3D11InputLayout> InputLayout;

		std::wstring VertFileName;
		std::wstring PixelFileName;

	};

	struct PhongTransformCB
	{
		XMMATRIX MVP;
		XMMATRIX Model;
	};

	struct PhongShadingCB
	{
		XMVECTOR LightPos;
		XMVECTOR CamPos;
	};

	struct Texture
	{
		ComPtr<ID3D11Texture2D> Texture2D;
		ComPtr<ID3D11ShaderResourceView> SRV;
		ComPtr<ID3D11UnorderedAccessView> UAV;
		UINT width, height;
		UINT levels;
	};

	struct Environment
	{
		Texture SpecularIBL;
		Texture DiffuseIBL;
		Texture BRDF_Lut;
	};

	struct Transform
	{
		Vector3 Position;
		Quaternion Rotation;
		Vector3 Scale;

		Transform()
		{
			Position = Vector3::Zero;
			Rotation = Quaternion::Identity;
			Scale = Vector3::One;
		}


	};

	enum class EScrollDirection
	{
		ScrollUp,
		ScrollDown
	};

	inline Vector3 EulerDegreesToRadians( Vector3 DegEuler )
	{
		Vector3 RadEuler = Vector3( XMConvertToRadians( DegEuler.x ), XMConvertToRadians( DegEuler.y ), XMConvertToRadians( DegEuler.z ) );
		return RadEuler;
	}

	inline Vector3 EulerRadiansToDegrees(Vector3 RadRuler)
	{
		Vector3 DegEuler = Vector3( XMConvertToDegrees( RadRuler.x ), XMConvertToDegrees( RadRuler.y ), XMConvertToDegrees( RadRuler.z ) );
		return DegEuler;
	}
}