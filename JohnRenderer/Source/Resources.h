#pragma once

#include "Types.h"

class JohnMesh;

namespace John
{
	template<typename T> static constexpr T NumMipmapLevels( T width, T height )
	{
		T levels = 1;
		while ( (width | height) >> levels ) {
			++levels;
		}
		return levels;
	}

	std::shared_ptr<JohnMesh> LoadMeshFromFile( const char* FileName );

	ShaderProgram CreateShaderProgram( ID3D11Device* device, const std::wstring& vsFile, const std::wstring& psFile,const std::string& VSEntryPoint = "main", const std::string& PSEntryPoint = "main");

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


	Texture CreateTexture( ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, UINT levels = 0 ) ;
	Texture CreateTexture( ID3D11Device* device, ID3D11DeviceContext* context, const std::shared_ptr<class Image>& image, DXGI_FORMAT format, UINT levels=0 ) ;
	Texture CreateTextureCube( ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, UINT levels = 0 ) ;

	Texture CreateDefaultBaseColor( ID3D11Device* device );
	Texture CreateDefaultNormal( ID3D11Device* device );
	Texture CreateDefaultRoughness( ID3D11Device* device );
	Texture CreateDefaultMetallic( ID3D11Device* device );

	Texture CreateDefaultTexture( ID3D11Device* device, uint16_t color, DXGI_FORMAT format );

	void CreateTextureUAV( ID3D11Device* device,Texture& texture, UINT mipSplice);

	FrameBuffer CreateFrameBuffer( ID3D11Device* device, UINT width, UINT height, UINT samples, DXGI_FORMAT colorFormat, DXGI_FORMAT depthStencilFormat ) ;

	Environment CreateEnvironmentFromFile( ID3D11Device* device, ID3D11DeviceContext* context, const char* EnvMapFile );

}