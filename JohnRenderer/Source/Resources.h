#pragma once

#include "Types.h"

class JohnMesh;

namespace John
{
	std::shared_ptr<JohnMesh> LoadMeshFromFile( const char* FileName );

	ShaderProgram CreateShaderProgram( ID3D11Device* device, const std::wstring& vsFile, const std::wstring& psFile);

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


}