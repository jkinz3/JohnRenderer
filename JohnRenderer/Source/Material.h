#pragma once
#include "Types.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;
using namespace DirectX;
struct TextureList
{
	John::Texture BaseColor;
	John::Texture Normal;
	John::Texture Roughness;
	John::Texture Metallic;
};



class Material
{
public:
	Material(ID3D11Device* device, ID3D11SamplerState* samplerState, EShaderProgram shaderProgram);

	void SetShaderProgram( EShaderProgram ShaderType );
	EShaderProgram GetShaderProgram() const { return m_ShaderProgram; }

	void Apply( ID3D11DeviceContext* context );

	void SetBaseColorMap( John::Texture NewColor );
	void SetNormalMap( John::Texture NewColor );
	void SetRoughnessMap( John::Texture NewColor );
	void SetMetallicMap( John::Texture NewColor );
	void SetSamplerState( ID3D11SamplerState* samplerState );

	void CreateDefaultTextures( ID3D11Device* device );

	void SetMatrices( XMMATRIX world, XMMATRIX view, XMMATRIX proj );

	John::Environment GetEnvironmentTextures() const;
	void SetEnvironmentTextures( John::Environment val );
	ID3D11SamplerState* GetBRDFSampler() const;
	void SetBRDFSampler( ID3D11SamplerState* val );
private:

	TextureList m_Textures;
	John::Environment m_EnvironmentTextures;


	EShaderProgram m_ShaderProgram;

	ID3D11SamplerState* m_SamplerState;
	ID3D11SamplerState* m_BRDFSampler;

	ComPtr<ID3D11Buffer> m_TransformCB;
	ComPtr<ID3D11Buffer> m_ShadingCB;



	XMMATRIX  m_World;
	XMMATRIX  m_View;
	XMMATRIX  m_Proj;

public:

	int m_AssetID;
};

