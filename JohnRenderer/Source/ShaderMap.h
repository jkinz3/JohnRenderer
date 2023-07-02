#pragma once
#include <map>
#include <memory.h>
#include "Utilities.h"
#include <functional>

using namespace Microsoft::WRL;
using namespace DX;

static HRESULT CompileShaderFromFile(_In_ LPCWSTR srcFile,  _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
{
	if ( !srcFile || !entryPoint || !profile || !blob )
		return E_INVALIDARG;

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(srcFile, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint, profile,
		flags, 0, &shaderBlob, &errorBlob);
	if ( FAILED(hr) )
	{
		if ( errorBlob )
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if ( shaderBlob )
			shaderBlob->Release();

		return hr;
	}

	*blob = shaderBlob;

	return hr;
}

class ShaderDescriptor
{
public:
	ShaderDescriptor(std::wstring ShaderName)
		:m_ShaderName(ShaderName)
	{

	}


	const std::wstring& GetShaderName() const { return m_ShaderName; }

	size_t GetHash();

private:

	bool bHashCalculated = false;
	std::wstring m_ShaderName;
	size_t m_Hash;
};

template<>
struct std::hash<ShaderDescriptor>
{
	size_t operator()(const ShaderDescriptor& descriptor) const
	{
		size_t res = 0;
		hash<wstring> myHash;
		res ^= myHash(descriptor.GetShaderName());
		return res;
	}
};

template<class TShaderWrapper>
class ShaderMap
{
public:
	static ShaderMap& Get()
	{
		static ShaderMap instance;
		return instance;
	}
	ShaderMap(ShaderMap const&) = delete;
	void operator=(ShaderMap const&) = delete;

	std::shared_ptr<TShaderWrapper> FindOrCreateShader(ShaderDescriptor descriptor)
	{
		size_t hash = descriptor.GetHash();
		std::map<size_t, std::shared_ptr<int>>::iterator result;
		std::shared_ptr<TShaderWrapper> wrap;
		typename std::map<size_t, std::shared_ptr<TShaderWrapper>>::iterator res = m_Shaders.find(hash);
		if(res != m_Shaders.end())
		{
			return (*res).second;
		}
		auto ret = TShaderWrapper::Create(descriptor);
		m_Shaders[hash] = ret;
		return ret;
	}



private:

	ShaderMap() {}
	std::map<size_t, std::shared_ptr<TShaderWrapper>> m_Shaders;

};


class PixelShaderWrapper
{
public:
	static std::shared_ptr<PixelShaderWrapper> Create(ShaderDescriptor descriptor)
	{
		struct make_shared_enabler : public PixelShaderWrapper
		{
			make_shared_enabler(ShaderDescriptor desc):
				PixelShaderWrapper(desc){}
		};

		auto result = std::make_shared<make_shared_enabler>(descriptor);

		//compile
		ID3DBlob* psBlob = nullptr;

		std::wstring path(L"Shaders/");
		std::wstring filePath = path + descriptor.GetShaderName ();

		DX::ThrowIfFailed(
			CompileShaderFromFile(filePath.c_str(), "main", "ps_5_5", &psBlob)
		);
		auto device = DeviceResources::Get().GetD3DDevice();
		DX::ThrowIfFailed(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, result->PixelShaderAddressOf()));

		return result;
	}

	ID3D11PixelShader** PixelShaderAddressOf() { return m_PixelShader.GetAddressOf(); }
	ID3D11PixelShader* GetPixelShader() { return m_PixelShader.Get(); }
private:
	PixelShaderWrapper(const ShaderDescriptor& descriptor)
		:m_Descriptor(descriptor)
	{
		
	}
	const ShaderDescriptor& m_Descriptor;
	ComPtr<ID3D11PixelShader> m_PixelShader;
};

class VertexShaderWrapper
{
public:
	static std::shared_ptr<VertexShaderWrapper> Create(ShaderDescriptor descriptor)
	{
		struct make_shared_enabler : public VertexShaderWrapper
		{
			make_shared_enabler(ShaderDescriptor desc) :
				VertexShaderWrapper(desc)
			{

			}
		};

		//compile
		auto result = std::make_shared<make_shared_enabler>(descriptor);

		ID3DBlob* vsBlob = nullptr;

		std::wstring path(L"Shaders/");
		std::wstring filePath = path + descriptor.GetShaderName ();
		auto device = DeviceResources::Get().GetD3DDevice();
		DX::ThrowIfFailed(
			CompileShaderFromFile(filePath.c_str(),  "main", "vs_5_0", &vsBlob)
		);

		DX::ThrowIfFailed(
			device->CreateVertexShader(
				vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, result->VertexShaderAddressOf()
		)

		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",		0,  DXGI_FORMAT_R32G32B32_FLOAT,	1,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",	0,  DXGI_FORMAT_R32G32_FLOAT,		2,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",		0,  DXGI_FORMAT_R32G32B32_FLOAT,	3,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BITANGENT",		0,  DXGI_FORMAT_R32G32B32_FLOAT,	4,	0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			device->CreateInputLayout(vertexDesc, 5, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), result->InputLayoutAddressOf())
		);
		return result;
	}

	ID3D11VertexShader** VertexShaderAddressOf() { return m_VertexShader.GetAddressOf(); }
	ID3D11VertexShader* GetVertexShader() { return m_VertexShader.Get(); }
	ID3D11InputLayout** InputLayoutAddressOf() { return m_InputLayout.GetAddressOf(); }
	ID3D11InputLayout* GetInputLayout() { return m_InputLayout.Get(); }

	


private:
	VertexShaderWrapper(const ShaderDescriptor& descriptor):
		m_Descriptor(descriptor)
	{

	}
	const ShaderDescriptor& m_Descriptor;
	ComPtr<ID3D11InputLayout> m_InputLayout;
	ComPtr<ID3D11VertexShader> m_VertexShader;
	std::unique_ptr<D3D_SHADER_MACRO> m_Defines = nullptr;
};