#pragma once
#include "Types.h"

using Microsoft::WRL::ComPtr;
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
	Material(ID3D11Device* device, ID3D11SamplerState* samplerState, John::ShaderProgram shaderProgram);

	void SetShaderProgram( John::ShaderProgram ShaderProgram );
	
	void Apply( ID3D11DeviceContext* context );

	void SetBaseColorMap( John::Texture NewColor );
	void SetNormalMap( John::Texture NewColor );
	void SetRoughnessMap( John::Texture NewColor );
	void SetMetallicMap( John::Texture NewColor );
	void SetSamplerState( ID3D11SamplerState* samplerState );

	void CreateDefaultTextures( ID3D11Device* device );

	void SetMatrices( XMMATRIX world, XMMATRIX view, XMMATRIX proj );

private:

	TextureList m_Textures;

	John::ShaderProgram m_ShaderProgram;

	ID3D11SamplerState* m_SamplerState;

	ComPtr<ID3D11Buffer> m_TransformCB;
	ComPtr<ID3D11Buffer> m_ShadingCB;


};

