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
		DirectX::XMVECTOR CameraPos;
	};

	struct Texture
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> UAV;
		UINT Width, Height;
		UINT Levels;

	};
}