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
}