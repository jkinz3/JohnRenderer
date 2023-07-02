#pragma once

#include "Types.h"
#include "Scene/Node.h"
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

	std::shared_ptr<Node> LoadSceneFromFile(const char* FileName);

	std::shared_ptr<JohnMesh> LoadMeshFromFile( const char* FileName );

	ShaderProgram CreateShaderProgram( const std::wstring& vsFile, const std::wstring& psFile,const std::string& VSEntryPoint = "main", const std::string& PSEntryPoint = "main");

	Microsoft::WRL::ComPtr<ID3D11ComputeShader> CreateComputeShader(  const std::wstring& csFile );

	Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateSamplerState(  D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode );

	template<typename T>
	Microsoft::WRL::ComPtr<ID3D11Buffer> CreateConstantBuffer( const T* data = nullptr)
	{
		auto device = DX::DeviceResources::Get().GetD3DDevice();
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


	Texture CreateTexture(  UINT width, UINT height, DXGI_FORMAT format, UINT levels = 0 ) ;
	Texture CreateTexture(   const std::shared_ptr<class Image>& image, DXGI_FORMAT format, UINT levels=0 ) ;
	Texture CreateTextureCube(  UINT width, UINT height, DXGI_FORMAT format, UINT levels = 0 ) ;

	Texture CreateDefaultBaseColor(  );
	Texture CreateDefaultNormal(  );
	Texture CreateDefaultRoughness(  );
	Texture CreateDefaultMetallic(  );

	Texture CreateDefaultTexture(  uint16_t color, DXGI_FORMAT format );

	void CreateTextureUAV( Texture& texture, UINT mipSplice);

	FrameBuffer CreateFrameBuffer(  UINT width, UINT height, UINT samples, DXGI_FORMAT colorFormat, DXGI_FORMAT depthStencilFormat ) ;

	Environment CreateEnvironmentFromFile(   const char* EnvMapFile );

}