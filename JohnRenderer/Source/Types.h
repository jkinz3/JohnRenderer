#pragma once
#include "pch.h"

using Microsoft::WRL::ComPtr;

using namespace DirectX::SimpleMath;
using namespace DirectX;

static constexpr int MaxPointLights = 5;

enum class EShaderProgram
{
	PBR,
	PHONG,
	Sky,
	LightSphere
};

struct Light
{
	XMVECTOR LightPos;
	XMVECTOR LightColor;
	float LightIntensity;

};

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

	struct FrameBuffer	
	{
		ComPtr<ID3D11Texture2D> ColorTexture;
		ComPtr<ID3D11Texture2D> DepthStencilTexture;
		ComPtr<ID3D11RenderTargetView> RTV;
		ComPtr<ID3D11ShaderResourceView> SRV;
		ComPtr<ID3D11DepthStencilView> DSV;
		UINT Width, Height;
		UINT Samples;
	};

	struct PhongTransformCB
	{
		XMMATRIX MVP;
		XMMATRIX Model;
		XMMATRIX Normal;
	};

	struct PhongShadingCB
	{
		XMVECTOR CamPos;
		Light Lights[MaxPointLights];
	};

	struct LightSphereTransformCB
	{
		XMMATRIX MVP;
		XMMATRIX Model;
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
		Vector3 Translation = { 0.f, 0.f, 0.f };
		Quaternion Rotation = Quaternion::Identity;
		Vector3 Scale = { 1.f, 1.f, 1.f };
		Vector3 RotationEuler = Vector3::Zero;

		Transform()
		{
			Translation = Vector3::Zero;
			Rotation = Quaternion::Identity;
			Scale = Vector3::One;
		}

		void SetTranslation(Vector3 NewTranslation) { Translation = NewTranslation; }
		void SetScale(Vector3 NewScale) { Scale = NewScale; }
		void SetRotation(Vector3 InRotation)
		{
			RotationEuler = InRotation;
			Rotation = Quaternion::CreateFromYawPitchRoll(RotationEuler.y, RotationEuler.x, RotationEuler.z);

		}
		void SetRotation(Quaternion InRotation)
		{
			Rotation = InRotation;
			RotationEuler = Rotation.ToEuler();

		}

		Vector3 GetTranslation() const { return Translation; }
		Vector3 GetScale() const { return Scale; }
		Quaternion GetRotation() const { return Rotation; }
		Vector3 GetRotationEuler() const { return RotationEuler; }

		void Reset()
		{
			Translation = { 0.f, 0.f, 0.f };
			Rotation = Quaternion::Identity;
			Scale = { 1.f, 1.f, 1.f };
		}


	};

	enum class EScrollDirection
	{
		ScrollUp,
		ScrollDown
	};

	enum class EPrimitiveType
	{
		Sphere,
		Plane,
		Cube,
		Torus
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

	enum class EAssetType
	{
		JohnMesh,
		JohnPrimitive

	};
}