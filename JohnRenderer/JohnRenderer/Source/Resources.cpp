#include "pch.h"
#include "JohnMesh.h"
#include "Resources.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/LogStream.hpp"
#include "assimp/DefaultLogger.hpp"



struct LogStream : public Assimp::LogStream
{
	static void initialize()
	{
		if ( Assimp::DefaultLogger::isNullLogger() )
		{
			Assimp::DefaultLogger::create( "", Assimp::Logger::VERBOSE );
			Assimp::DefaultLogger::get()->attachStream( new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn );
		}
	}

	void write( const char* message ) override
	{
		std::fprintf( stderr, "Assimp: %s", message );
	}
};

namespace John
{

	std::shared_ptr<JohnMesh> LoadMeshFromFile( const char* FileName )
	{
		LogStream::initialize();

		std::printf( "Loading Mesh: %s\n", FileName );

		std::shared_ptr<JohnMesh> newMesh = std::make_shared<JohnMesh>();
		Assimp::Importer import;

		const unsigned int importFlags =
			aiProcess_Triangulate |
			aiProcess_FlipUVs |
			aiProcess_CalcTangentSpace |
			aiProcess_JoinIdenticalVertices;

		const aiScene* scene = import.ReadFile( FileName, importFlags );
		if ( !scene )
		{
			std::string output = std::string( "Failed to load mesh: %s", FileName );
			throw std::runtime_error( output );
		}

		aiMesh* mesh = scene->mMeshes[0];
		assert( mesh->HasPositions() );
		assert( mesh->HasNormals() );

		for ( size_t i = 0; i < mesh->mNumVertices; ++i )
		{
			Vertex vertex;
			vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
			if ( mesh->HasTangentsAndBitangents() )
			{
				vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
				vertex.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			}
			if ( mesh->HasTextureCoords( 0 ) )
			{
				vertex.TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			}
			newMesh->GetVertices()->push_back( vertex );
		}

		for ( size_t i = 0; i < mesh->mNumFaces; ++i )
		{
			assert( mesh->mFaces[i].mNumIndices == 3 );
			newMesh->GetFaces()->push_back( { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] } );
		}

		return newMesh;
	}

	ShaderProgram CreateShaderProgram( ID3D11Device* device, const std::wstring& vsFile, const std::wstring& psFile )
	{
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		flags |= D3DCOMPILE_DEBUG;
		flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		ComPtr<ID3DBlob> vsShaderBlob;
		ComPtr<ID3DBlob> psShaderBlob;
		ComPtr<ID3DBlob> errorBlob;

		DX::ThrowIfFailed(
			D3DCompileFromFile(
				vsFile.c_str(),
				nullptr,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				"main",
				"vs_5_0",
				flags,
				0,
				vsShaderBlob.GetAddressOf(),
				errorBlob.GetAddressOf()
			)
		);

		DX::ThrowIfFailed(
			D3DCompileFromFile(
				psFile.c_str(),
				nullptr,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				"main",
				"ps_5_0",
				flags,
				0,
				psShaderBlob.GetAddressOf(),
				errorBlob.GetAddressOf()
			)
		);

		John::ShaderProgram program;

		program.VertFileName = vsFile;
		program.PixelFileName = psFile;

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

		const std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc =
		{
			{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		DX::ThrowIfFailed(
			device->CreateInputLayout(
				layoutDesc.data(), (UINT)layoutDesc.size(), vsShaderBlob->GetBufferPointer(), vsShaderBlob->GetBufferSize(), program.InputLayout.ReleaseAndGetAddressOf()
			)
		);


		return program;
	}

}

