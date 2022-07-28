#include "pch.h"
#include "Utilities.h"
#include "JohnMesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/LogStream.hpp"
#include "assimp/DefaultLogger.hpp"

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
			//	vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
			//	vertex.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
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

	John::ShaderProgram CreateShaderProgram(const wchar_t* vertFile, const wchar_t* pixelFile, const std::vector<D3D11_INPUT_ELEMENT_DESC>* inputLayoutDesc, ID3D11Device* device)
	{
		
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		flags |= D3DCOMPILE_DEBUG;
		flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		Microsoft::WRL::ComPtr<ID3DBlob> vsShaderBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> psShaderBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;


		DX::ThrowIfFailed(
			D3DCompileFromFile(
				vertFile, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", flags, 0, vsShaderBlob.GetAddressOf(), errorBlob.GetAddressOf() 
			)
		);

		DX::ThrowIfFailed(
			D3DCompileFromFile(
				pixelFile, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", flags, 0, psShaderBlob.GetAddressOf(), errorBlob.GetAddressOf()
			)
		);

		ShaderProgram program;

		program.VertFileName = std::wstring(vertFile);
		program.PixelFileName = std::wstring(pixelFile);;
		
		DX::ThrowIfFailed(
			device->CreateVertexShader(
				vsShaderBlob->GetBufferPointer(), vsShaderBlob->GetBufferSize(), nullptr, program.VertexShader.ReleaseAndGetAddressOf()
			)
		);
		DX::ThrowIfFailed(
			device->CreatePixelShader(
				psShaderBlob->GetBufferPointer(), psShaderBlob->GetBufferSize(), nullptr, program.PixelShader.ReleaseAndGetAddressOf()
			)
		);

		if(inputLayoutDesc)
		{
			DX::ThrowIfFailed(
				device->CreateInputLayout(
					inputLayoutDesc->data(), (UINT)inputLayoutDesc->size(), vsShaderBlob->GetBufferPointer(), vsShaderBlob->GetBufferSize(), program.InputLayout.ReleaseAndGetAddressOf()
				)
			);

		}

		return program;



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

}