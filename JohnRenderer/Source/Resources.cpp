#include "pch.h"
#include "JohnMesh.h"
#include "Resources.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/LogStream.hpp"
#include "assimp/DefaultLogger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



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

	ShaderProgram CreateShaderProgram( ID3D11Device* device, const std::wstring& vsFile, const std::wstring& psFile, const std::string& VSEntryPoint , const std::string& PSEntryPoint )
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
				VSEntryPoint.c_str(),
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
				PSEntryPoint.c_str(),
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

	Microsoft::WRL::ComPtr<ID3D11ComputeShader> CreateComputeShader( ID3D11Device* device, const std::wstring& csFile )
	{
		ComPtr<ID3D11ComputeShader> shader;

		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		flags |= D3DCOMPILE_DEBUG;
		flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		ComPtr<ID3DBlob> csShaderBlob;
		ComPtr<ID3DBlob> errorBlob;

		DX::ThrowIfFailed(
			D3DCompileFromFile(
				csFile.c_str(),
				nullptr,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				"main",
				"cs_5_0",
				flags,
				0,
				csShaderBlob.GetAddressOf(),
				errorBlob.GetAddressOf()
			)
		);

		DX::ThrowIfFailed(
			device->CreateComputeShader( csShaderBlob->GetBufferPointer(), csShaderBlob->GetBufferSize(), nullptr, shader.ReleaseAndGetAddressOf() )
		);

		return shader;
	}

	Microsoft::WRL::ComPtr<ID3D11SamplerState> CreateSamplerState( ID3D11Device* device, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode )
	{
		D3D11_SAMPLER_DESC desc = {};
		desc.Filter = filter;
		desc.AddressU = addressMode;
		desc.AddressV = addressMode;
		desc.AddressW = addressMode;
		desc.MaxAnisotropy = (filter == D3D11_FILTER_ANISOTROPIC) ? D3D11_REQ_MAXANISOTROPY : 1;
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;

		ComPtr<ID3D11SamplerState> samplerState;
		DX::ThrowIfFailed(
			device->CreateSamplerState(&desc, samplerState.ReleaseAndGetAddressOf())
		);

		return samplerState;

	}

	John::Texture CreateTexture( ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, UINT levels /*= 0 */ )
	{
		Texture texture;
		texture.width = width;
		texture.height = height;
		texture.levels = (levels > 0) ? levels : NumMipmapLevels( width, height );

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = levels;
		texDesc.ArraySize = 1;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		if(levels == 0)
		{
			texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		DX::ThrowIfFailed(
			device->CreateTexture2D(&texDesc, nullptr, texture.Texture2D.ReleaseAndGetAddressOf())
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;
		DX::ThrowIfFailed(
			device->CreateShaderResourceView(texture.Texture2D.Get(), &srvDesc, texture.SRV.ReleaseAndGetAddressOf())
		);

		return texture;
	}


	John::Texture CreateTexture( ID3D11Device* device, ID3D11DeviceContext* context, const std::shared_ptr<class Image>& image, DXGI_FORMAT format, UINT levels )
	{
		Texture texture = CreateTexture(device,  image->width(), image->height(), format, levels );
		context->UpdateSubresource( texture.Texture2D.Get(), 0, nullptr, image->pixels<void>(), image->pitch(), 0 );
		if(levels == 0)
		{
			context->GenerateMips( texture.SRV.Get() );
		}
		return texture;
	}

	John::Texture CreateTextureCube( ID3D11Device* device,UINT width, UINT height, DXGI_FORMAT format, UINT levels /*= 0 */ )
	{
		Texture texture;
		texture.width = width;
		texture.height = height;
		texture.levels = (levels > 0) ? levels : NumMipmapLevels( width, height );

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
		if(levels == 0)
		{
			texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			texDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		DX::ThrowIfFailed(
			device->CreateTexture2D(&texDesc, nullptr, texture.Texture2D.ReleaseAndGetAddressOf())
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = -1;

		DX::ThrowIfFailed(
			device->CreateShaderResourceView(
				texture.Texture2D.Get(), &srvDesc, texture.SRV.ReleaseAndGetAddressOf()
			)
		);

		return texture;
	}

	John::Texture CreateDefaultBaseColor( ID3D11Device* device )
	{
		return CreateDefaultTexture( device, 0x000000, DXGI_FORMAT_R8G8B8A8_UNORM );
	}

	Texture CreateDefaultNormal( ID3D11Device * device )
	{
		return CreateDefaultTexture( device, 0x7f7f, DXGI_FORMAT_R8G8_UNORM );
	}

	Texture CreateDefaultRoughness( ID3D11Device * device )
	{
		return CreateDefaultTexture( device, 0x7f, DXGI_FORMAT_R8_UNORM );
	}

	Texture CreateDefaultMetallic( ID3D11Device * device )
	{
		return CreateDefaultTexture( device, 0x00, DXGI_FORMAT_R8_UNORM );
	}


	John::Texture CreateDefaultTexture( ID3D11Device* device, uint16_t color , DXGI_FORMAT format)
	{


		D3D11_SUBRESOURCE_DATA initData = { &color, sizeof( uint16_t ), 0 };

		Texture DefaultTexture = {};

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = desc.Height = desc.MipLevels = desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		DX::ThrowIfFailed(
			device->CreateTexture2D( &desc, &initData, DefaultTexture.Texture2D.ReleaseAndGetAddressOf() )
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVdesc = {};
		SRVdesc.Format = desc.Format;
		SRVdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVdesc.Texture2D.MipLevels = 1;

		DX::ThrowIfFailed(
			device->CreateShaderResourceView( DefaultTexture.Texture2D.Get(), &SRVdesc, DefaultTexture.SRV.ReleaseAndGetAddressOf() )
		);

		DefaultTexture.height = DefaultTexture.width = DefaultTexture.levels = desc.Width;

		return DefaultTexture;
	}

	void CreateTextureUAV( ID3D11Device* device, Texture& texture, UINT mipSplice )
	{
		assert( texture.Texture2D);

		D3D11_TEXTURE2D_DESC texDesc = {};
		texture.Texture2D->GetDesc( &texDesc );

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = texDesc.Format;

		if(texDesc.ArraySize == 1)
		{
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = mipSplice;

		}
		else
		{
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.MipSlice = mipSplice;
			uavDesc.Texture2DArray.FirstArraySlice = 0;
			uavDesc.Texture2DArray.ArraySize = texDesc.ArraySize;
		}

		DX::ThrowIfFailed(
			device->CreateUnorderedAccessView(texture.Texture2D.Get(), &uavDesc, texture.UAV.ReleaseAndGetAddressOf())
		);

	}

	John::FrameBuffer CreateFrameBuffer( ID3D11Device* device, UINT width, UINT height, UINT samples, DXGI_FORMAT colorFormat, DXGI_FORMAT depthStencilFormat ) 
	{
		FrameBuffer fb;
		fb.Width = width;
		fb.Height = height;
		fb.Samples = samples;

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = samples;

		if(colorFormat != DXGI_FORMAT_UNKNOWN)
		{
			desc.Format = colorFormat;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET;
			if(samples <= 1)
			{
				desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
			}
			DX::ThrowIfFailed(
				device->CreateTexture2D(&desc, nullptr, fb.ColorTexture.ReleaseAndGetAddressOf())
			);

			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = desc.Format;
			rtvDesc.ViewDimension = (samples > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
			DX::ThrowIfFailed(
				device->CreateRenderTargetView(fb.ColorTexture.Get(), &rtvDesc, fb.RTV.ReleaseAndGetAddressOf())
			);

			if(samples <= 1)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = desc.Format;
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.MipLevels = 1;
				DX::ThrowIfFailed(
					device->CreateShaderResourceView(fb.ColorTexture.Get(), &srvDesc, fb.SRV.ReleaseAndGetAddressOf())
				);
			}



		}

		if(depthStencilFormat != DXGI_FORMAT_UNKNOWN)
		{
			desc.Format = depthStencilFormat;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			DX::ThrowIfFailed(
				device->CreateTexture2D(&desc, nullptr, fb.DepthStencilTexture.ReleaseAndGetAddressOf())
			);

			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = desc.Format;
			dsvDesc.ViewDimension = (samples > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
			DX::ThrowIfFailed(
				device->CreateDepthStencilView( fb.DepthStencilTexture.Get(), &dsvDesc, fb.DSV.ReleaseAndGetAddressOf())
			);
		}
		return fb;
	}

	John::Environment CreateEnvironmentFromFile( ID3D11Device* device, ID3D11DeviceContext* context, const char* EnvMapFile )
	{
		ID3D11UnorderedAccessView* const nullUAV[] = { nullptr };
		ID3D11Buffer* const nullBuffer[] = { nullptr };


		ComPtr<ID3D11SamplerState> ComputeSampler = CreateSamplerState( device, D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP );


		John::Environment environment;
		{
			//unfiltered env cube map (TEMP DO NOT USE TO DRAW)
			Texture envMapUnfiltered = CreateTextureCube( device, 1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT );
			CreateTextureUAV( device, envMapUnfiltered, 0 );

			{
				//load and convert equirectangular env map to cubemap
				ComPtr<ID3D11ComputeShader> equirectToCubemapProgram = CreateComputeShader( device, L"Shaders/Compute/equirect2cube.hlsl" );
				Texture envTextureEquirect = CreateTexture( device,context,  Image::fromFile(EnvMapFile), DXGI_FORMAT_R32G32B32A32_FLOAT , 1 );

				context->CSSetShaderResources( 0, 1, envTextureEquirect.SRV.GetAddressOf() );
				context->CSSetUnorderedAccessViews( 0, 1, envMapUnfiltered.UAV.GetAddressOf(), nullptr );
				context->CSSetSamplers( 0, 1, ComputeSampler.GetAddressOf() );
				context->CSSetShader( equirectToCubemapProgram.Get(), nullptr, 0 );
				context->Dispatch( envMapUnfiltered.width / 32, envMapUnfiltered.height / 32, 6 );
				context->CSSetUnorderedAccessViews( 0, 1, nullUAV, nullptr );

			}

			context->GenerateMips( envMapUnfiltered.SRV.Get() );

			{
				//compute pre-filtered spec env map
				struct SpecularMapFilterSettingsCB
				{
					float roughness;
					float padding[3];
				};
				ComPtr<ID3D11ComputeShader> spMapProgram = CreateComputeShader( device, L"Shaders/Compute/SPMap.hlsl" );
				ComPtr<ID3D11Buffer> spMapCB = CreateConstantBuffer<SpecularMapFilterSettingsCB>( device );

				environment.SpecularIBL = CreateTextureCube( device, 1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT );

				for(int arraySlice=0; arraySlice<6; ++arraySlice)
				{
					const UINT subresourceIndex = D3D11CalcSubresource( 0, arraySlice, environment.SpecularIBL.levels );
					context->CopySubresourceRegion( environment.SpecularIBL.Texture2D.Get(), subresourceIndex, 0, 0, 0, envMapUnfiltered.Texture2D.Get(), subresourceIndex, nullptr );
				}

				context->CSSetShaderResources( 0, 1, envMapUnfiltered.SRV.GetAddressOf() );
				context->CSSetSamplers( 0, 1, ComputeSampler.GetAddressOf() );
				context->CSSetShader( spMapProgram.Get(), nullptr, 0 );

				//prefilter all the mips
				const float deltaRoughness = 1.f / std::max( float(environment.SpecularIBL.levels )- 1, 1.f );
				for(UINT level=1, size=512; level<environment.SpecularIBL.levels; ++level, size/2)
				{
					const UINT numGroups = std::max<UINT>( 1, size / 32 );
					CreateTextureUAV( device, environment.SpecularIBL, level );

					const SpecularMapFilterSettingsCB spMapConstants = { level * deltaRoughness };
					context->UpdateSubresource( spMapCB.Get(), 0, nullptr, &spMapConstants, 0, 0 );

					context->CSSetConstantBuffers( 0, 1, spMapCB.GetAddressOf() );
					context->CSSetUnorderedAccessViews( 0, 1, environment.SpecularIBL.UAV.GetAddressOf(), nullptr );
					context->Dispatch( numGroups, numGroups, 6 );
				}
				context->CSSetConstantBuffers( 0, 1, nullBuffer );
				context->CSSetUnorderedAccessViews( 0, 1, nullUAV, nullptr);

			}
		}
		{
			//compute diffuse ibl
			ComPtr<ID3D11ComputeShader> irMapProgram = CreateComputeShader( device, L"Shaders/Compute/irmap.hlsl" );
			environment.DiffuseIBL = CreateTextureCube(device, 32, 32, DXGI_FORMAT_R16G16B16A16_FLOAT, 1);
			CreateTextureUAV( device, environment.DiffuseIBL, 0 );

			context->CSSetShaderResources( 0, 1, environment.SpecularIBL.SRV.GetAddressOf() );
			context->CSSetSamplers( 0, 1, ComputeSampler.GetAddressOf() );
			context->CSSetUnorderedAccessViews( 0, 1, environment.DiffuseIBL.UAV.GetAddressOf(), nullptr );
			context->CSSetShader( irMapProgram.Get(), nullptr, 0 );
			context->Dispatch( environment.DiffuseIBL.width / 32, environment.DiffuseIBL.height / 32, 6 );
			context->CSSetUnorderedAccessViews( 0, 1, nullUAV, nullptr );
		}

		{
			//compute cook torrance brdf 2D LUT
			ComPtr<ID3D11ComputeShader> spBRDFShader = CreateComputeShader( device, L"Shaders/Compute/SPBRDF.hlsl" );

			environment.BRDF_Lut = CreateTexture( device, 256, 256, DXGI_FORMAT_R16G16B16A16_FLOAT, 1 );
			CreateTextureUAV(device,  environment.BRDF_Lut, 0 );

			context->CSSetUnorderedAccessViews( 0, 1, environment.BRDF_Lut.UAV.GetAddressOf(), nullptr );
			context->CSSetShader( spBRDFShader.Get(), nullptr, 0 );
			context->Dispatch( environment.BRDF_Lut.width / 32, environment.BRDF_Lut.height / 32, 1 );
			context->CSSetUnorderedAccessViews( 0, 1, nullUAV, nullptr );
		}

		return environment;
	}

}

