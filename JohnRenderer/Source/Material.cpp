#include "pch.h"
#include "Material.h"
#include "Resources.h"

Material::Material(ID3D11Device* device, ID3D11SamplerState* samplerState, John::ShaderProgram shaderProgram)
{
	CreateDefaultTextures( device );
	m_ShaderProgram = shaderProgram;
	m_SamplerState = samplerState;
}

void Material::SetShaderProgram( John::ShaderProgram ShaderProgram )
{
	m_ShaderProgram = ShaderProgram;
}

void Material::Apply( ID3D11DeviceContext* context )
{

}

void Material::SetBaseColorMap( John::Texture NewColor )
{
	m_Textures.BaseColor = NewColor;
}

void Material::SetNormalMap( John::Texture NewNormal )
{
		m_Textures.Normal = NewNormal;
}

void Material::SetRoughnessMap( John::Texture NewRoughness)
{
		m_Textures.Roughness = NewRoughness;
}

void Material::SetMetallicMap( John::Texture NewMetallic )
{
		m_Textures.Metallic= NewMetallic;
}

void Material::SetSamplerState( ID3D11SamplerState* samplerState )
{
	m_SamplerState = samplerState;
}

void Material::CreateDefaultTextures( ID3D11Device* device )
{
	m_Textures.BaseColor = John::CreateDefaultBaseColor( device );
	m_Textures.Normal= John::CreateDefaultNormal( device );
	m_Textures.Roughness= John::CreateDefaultRoughness( device );
	m_Textures.Metallic = John::CreateDefaultMetallic( device );
}

void Material::SetMatrices( XMMATRIX world, XMMATRIX view, XMMATRIX proj )
{

}
