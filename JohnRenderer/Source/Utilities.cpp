#include "pch.h"
#include "Utilities.h"
#include "JohnMesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/LogStream.hpp"
#include "assimp/DefaultLogger.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace 
{
	const unsigned int AssimpImportFlags =
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_SortByPType |
		aiProcess_PreTransformVertices |
		aiProcess_GenNormals |
		aiProcess_GenUVCoords |
		aiProcess_OptimizeMeshes |
		aiProcess_Debone |
		aiProcess_ValidateDataStructure |
		aiProcess_FlipUVs;
}


struct LogStream : public Assimp::LogStream
{
	static void initialize()
	{
		if (Assimp::DefaultLogger::isNullLogger())
		{
			Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
			Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
		}
	}

	void write(const char* message) override
	{
		std::fprintf(stderr, "Assimp: %s", message);
	}
};

namespace John
{

	std::shared_ptr<JohnMesh> LoadMeshFromFile(const char* FileName)
	{
		LogStream::initialize();

		std::printf("Loading Mesh: %s\n", FileName);

		std::shared_ptr<JohnMesh> newMesh = std::make_shared<JohnMesh>();
		Assimp::Importer import;

		const aiScene* scene = import.ReadFile(FileName, AssimpImportFlags);
		if(!scene || !scene->HasMeshes())
		{
			std::string output = std::string("Failed to load Mesh File: %s", FileName);
			throw std::runtime_error(output);
		}

		aiMesh* mesh = scene->mMeshes[0];
		assert(mesh->HasPositions());
		assert(mesh->HasNormals());

		newMesh->GetVertices()->reserve(mesh->mNumVertices);

		for (size_t i = 0; i < newMesh->GetVertices()->capacity(); ++i)
		{
			Vertex vertex;
			vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
			if (mesh->HasTangentsAndBitangents())
			{
				vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
				vertex.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			}
			if (mesh->HasTextureCoords(0))
			{
				vertex.TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			}
			newMesh->GetVertices()->push_back(vertex);

		}

		newMesh->GetFaces()->reserve(mesh->mNumFaces);
		for (size_t i = 0; i < newMesh->GetFaces()->capacity(); ++i)
		{
			assert(mesh->mFaces[i].mNumIndices == 3);
			newMesh->GetFaces()->push_back({ mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] });
		}

		return newMesh;

	}



	John::ShaderProgram CreateShaderProgram(const Microsoft::WRL::ComPtr<ID3DBlob> vsByteCode, const Microsoft::WRL::ComPtr<ID3DBlob> psByteCode, const std::wstring& vsFile, const std::wstring& psFile, const std::vector<D3D11_INPUT_ELEMENT_DESC>* inputLayoutDesc, ID3D11Device* device)
	{
		ShaderProgram program;

		program.VertFileName = std::wstring(vsFile.c_str());
		program.PixelFileName = std::wstring(psFile.c_str());;

		DX::ThrowIfFailed(
			device->CreateVertexShader(
				vsByteCode->GetBufferPointer(), vsByteCode->GetBufferSize(), nullptr, program.VertexShader.ReleaseAndGetAddressOf()
			)
		);
		DX::ThrowIfFailed(
			device->CreatePixelShader(
				psByteCode->GetBufferPointer(), psByteCode->GetBufferSize(), nullptr, program.PixelShader.ReleaseAndGetAddressOf()
			)
		);

		if (inputLayoutDesc)
		{
			DX::ThrowIfFailed(
				device->CreateInputLayout(
					inputLayoutDesc->data(), (UINT)inputLayoutDesc->size(), vsByteCode->GetBufferPointer(), vsByteCode->GetBufferSize(), program.InputLayout.ReleaseAndGetAddressOf()
				)
			);

		}

		return program;


	}

	John::ComputeProgram CreateComputeProgram(const Microsoft::WRL::ComPtr<ID3DBlob>& csByteCode, ID3D11Device* device)
	{
		ComputeProgram program;

		DX::ThrowIfFailed(
			device->CreateComputeShader(csByteCode->GetBufferPointer(), csByteCode->GetBufferSize(), nullptr, &program.ComputeShader)
		);

		return program;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring& filename, const std::string& entryPoint, const std::string& profile)
	{

		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		flags |= D3DCOMPILE_DEBUG;
		flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		Microsoft::WRL::ComPtr<ID3DBlob> shader;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;


		DX::ThrowIfFailed(
			D3DCompileFromFile(
				filename.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str() , profile.c_str(), flags, 0, shader.GetAddressOf(), errorBlob.GetAddressOf()
			)
		);

		return shader;
	}

	Microsoft::WRL::ComPtr<ID3D11Buffer> CreateConstantBuffer(const void* data, UINT size, ID3D11Device* device)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = static_cast<UINT> (size);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		D3D11_SUBRESOURCE_DATA bufferData;
		bufferData.pSysMem = data;

		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
		const D3D11_SUBRESOURCE_DATA* bufferDataPtr = data ? &bufferData : nullptr;
		DX::ThrowIfFailed(
			device->CreateBuffer(&desc, bufferDataPtr, buffer.ReleaseAndGetAddressOf())
		);

		return buffer;
	}

	Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateSamplerState(ID3D11Device* device, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode)
	{
		D3D11_SAMPLER_DESC desc = {};
		desc.Filter = filter;
		desc.AddressU = addressMode;
		desc.AddressV = addressMode;
		desc.AddressW = addressMode;

		desc.MaxAnisotropy = (filter == D3D11_FILTER_ANISOTROPIC) ? D3D11_REQ_MAXANISOTROPY : 1;
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
		DX::ThrowIfFailed(
			device->CreateSamplerState(
				&desc,
				samplerState.ReleaseAndGetAddressOf()
			)
		);
		return samplerState;
	}

	John::Texture CreateTexture(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, UINT levels)
	{
		
		Texture texture;
		texture.Width = width;
		texture.Height = height;
		texture.Levels = (levels > 0) ? levels : John::NumMipMapLevels(width, height);

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = levels;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		if (levels == 0)
		{
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
		DX::ThrowIfFailed(
			device->CreateTexture2D(&desc, nullptr, &texture.texture)
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;

		DX::ThrowIfFailed(
			device->CreateShaderResourceView(texture.texture.Get(), &srvDesc, &texture.SRV)
		);

		return texture;

	}

	John::Texture CreateTexture(ID3D11Device* device, ID3D11DeviceContext* context, const std::shared_ptr<Image>& image, DXGI_FORMAT format, UINT levels)
	{
		Texture texture = John::CreateTexture(device, image->GetWidth(), image->GetHeight(), format, levels);
		context->UpdateSubresource(texture.texture.Get(), 0, nullptr, image->GetPixels<void>(), image->GetPitch(), 0);
		if (levels == 0)
		{
			context->GenerateMips(texture.SRV.Get());
		}
		return texture;
	}

	John::Texture CreateDefaultNormalTexture( ID3D11Device* device )
	{
		Texture defaultNormal;
		static const uint32_t pixel = 0x7f7f;
		
		D3D11_SUBRESOURCE_DATA initData = { &pixel, sizeof( uint32_t ), 0 };

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = desc.Height = desc.MipLevels = desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		DX::ThrowIfFailed(device->CreateTexture2D(&desc, &initData, &defaultNormal.texture));

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = desc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		DX::ThrowIfFailed(
			device->CreateShaderResourceView( defaultNormal.texture.Get(), &SRVDesc, defaultNormal.SRV.ReleaseAndGetAddressOf() )
		);

		return defaultNormal;
	}

	John::Texture CreateTextureCube( ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, UINT levels )
	{

		Texture NewTexture;
		NewTexture.Width = width;
		NewTexture.Height = height;
		NewTexture.Levels = (levels > 0) ? levels : John::NumMipMapLevels(width, height);

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = levels;
		texDesc.ArraySize = 6;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		if (levels == 0)
		{
			texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			texDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		DX::ThrowIfFailed(
			device->CreateTexture2D(&texDesc, nullptr, NewTexture.texture.ReleaseAndGetAddressOf())
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;

		DX::ThrowIfFailed(
			device->CreateShaderResourceView(NewTexture.texture.Get(), &srvDesc, &NewTexture.SRV)
		);

		return NewTexture;
	}

	void CreateTextureUAV(Texture& texture, UINT mipSlice, ID3D11Device* device)
	{
		assert(texture.texture);

		D3D11_TEXTURE2D_DESC desc;
		texture.texture->GetDesc(&desc);

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = desc.Format;
		if (desc.ArraySize == 1)
		{
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = mipSlice;
		}
		else
		{
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.MipSlice = mipSlice;
			uavDesc.Texture2DArray.FirstArraySlice = 0;
			uavDesc.Texture2DArray.ArraySize = desc.ArraySize;
		}

		DX::ThrowIfFailed(
			device->CreateUnorderedAccessView(texture.texture.Get(), &uavDesc, &texture.UAV)
		);
	}

}