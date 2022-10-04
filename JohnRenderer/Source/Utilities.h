#pragma once
#include "Types.h"
#include "Image.h"
class JohnMesh;



namespace John
{
	std::shared_ptr<JohnMesh> LoadMeshFromFile(const char* FileName);

	Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring& filename, const std::string& entryPoint, const std::string& profile);

	ShaderProgram CreateShaderProgram(const Microsoft::WRL::ComPtr<ID3DBlob> vsByteCode,const Microsoft::WRL::ComPtr<ID3DBlob> psByteCode, const std::wstring& vsFile, const std::wstring& psFile, const std::vector<D3D11_INPUT_ELEMENT_DESC>* inputLayoutDesc, ID3D11Device* device);

	ComputeProgram CreateComputeProgram(const Microsoft::WRL::ComPtr<ID3DBlob>& csByteCode, ID3D11Device* device);

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

	Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateSamplerState(ID3D11Device* device, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode);

	Texture CreateTexture(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, UINT levels);

	Texture CreateTextureCube(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, UINT levels = 0);

	Texture CreateTexture(ID3D11Device* device, ID3D11DeviceContext* context, const std::shared_ptr<John::Image>& image, DXGI_FORMAT format, UINT levels);

	Texture CreateDefaultNormalTexture( ID3D11Device* device );

	void CreateTextureUAV(Texture& texture, UINT mipSlice, ID3D11Device* device);

	template<typename T> static constexpr T NumMipMapLevels(T width, T height)
	{
		T levels = 1;
		while ((width|height) >> levels)
		{
			++levels;
		}
		return levels;
	}

}
