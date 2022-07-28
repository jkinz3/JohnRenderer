#pragma once
#include "Types.h"

class JohnMesh;



namespace John
{
	std::shared_ptr<JohnMesh> LoadMeshFromFile(const char* FileName);

	ShaderProgram CreateShaderProgram(const wchar_t* vertFile, const wchar_t* pixelFile,  const std::vector<D3D11_INPUT_ELEMENT_DESC>* inputLayoutDesc, ID3D11Device* device);

	template<typename T> static constexpr T RoundToPowerOfTwo(T value, int POT)
	{
		return (value + POT - 1) & -POT;
	}

	Microsoft::WRL::ComPtr<ID3D11Buffer> CreateConstantBuffer(const void* data, UINT size, ID3D11Device* device);
	template<typename T> Microsoft::WRL::ComPtr<ID3D11Buffer> CreateConstantBuffer(ID3D11Device* device, const T* data = nullptr)
	{
		static_assert(sizeof(T) == RoundToPowerOfTwo(sizeof(T), 16));
		return CreateConstantBuffer(data, sizeof(T), device);
	}


}