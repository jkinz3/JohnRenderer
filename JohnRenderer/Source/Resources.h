#pragma once

#include "Types.h"

class JohnMesh;

namespace John
{
	std::shared_ptr<JohnMesh> LoadMeshFromFile( const char* FileName );

	ShaderProgram CreateShaderProgram( ID3D11Device* device, const std::wstring& vsFile, const std::wstring& psFile);

	Microsoft::WRL::ComPtr<ID3D11ComputeShader> CreateComputeShader( ID3D11Device* device, const std::wstring& csFile );

	Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateSamplerState( ID3D11Device* device, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode );

	template<typename T>
	Microsoft::WRL::ComPtr<ID3D11Buffer> CreateConstantBuffer(ID3D11Device* device, const T* data = nullptr)
	{
		UINT size = sizeof( T );

		D3D11_BUFFER_DESC desc = {};

		desc.ByteWidth = size;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		D3D11_SUBRESOURCE_DATA bufferData = {};
		bufferData.pSysMem = data;

		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;

		DX::ThrowIfFailed(
			device->CreateBuffer(&desc, nullptr, buffer.ReleaseAndGetAddressOf())
		);

		return buffer;

	}

	struct Image
	{
		std::unique_ptr<unsigned char> pixels;

		template<typename T>
		const T* GetPixels() const
		{
			return reinterpret_cast<const T*> (pixels.get());
		}
	};

	template<typename T> static constexpr T NumMipMapLevels(T width, T height)
	{
		T levels = 1;
		while((width|height) >> levels)
		{
			++levels;
		}
		return levels;
	}

	Texture CreateTexture( ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, UINT levels = 0 );
	Texture CreateTextureFromFile(ID3D11Device* device, ID3D11DeviceContext* context, const char* ImageFile, DXGI_FORMAT format , UINT levels = 0 );
	Texture CreateTextureCube(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, UINT levels = 0 );

	void CreateTextureUAV( ID3D11Device* device,Texture& texture, UINT mipSplice);


	Environment CreateEnvironmentFromFile( ID3D11Device* device, ID3D11DeviceContext* context, const char* EnvMapFile );

}