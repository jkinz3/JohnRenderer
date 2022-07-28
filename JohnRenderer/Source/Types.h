#pragma once
#include "pch.h"

namespace John
{
	struct ShaderProgram
	{
		Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
		std::wstring VertFileName;
		std::wstring PixelFileName;
	};

	struct TransformCB
	{
		DirectX::XMMATRIX MVP;
		DirectX::XMMATRIX Model;
	};

	struct ShadingCB
	{
		DirectX::XMVECTOR LightPos;
	};
}