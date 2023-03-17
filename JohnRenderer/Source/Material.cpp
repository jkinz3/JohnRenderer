#include "pch.h"
#include "Material.h"
#include "Resources.h"

Material::Material(ID3D11Device* device, ID3D11SamplerState* samplerState, EShaderProgram shaderProgram)
{
	CreateDefaultTextures( device );
	m_ShaderProgram = shaderProgram;
	m_SamplerState = samplerState;
}

void Material::SetShaderProgram( EShaderProgram ShaderType )
{
	m_ShaderProgram = ShaderType;
}

void Material::Apply( ID3D11DeviceContext* context )
{
	ID3D11ShaderResourceView* const srvs[] =
	{
		m_EnvironmentTextures.SpecularIBL.SRV.Get(),
		m_EnvironmentTextures.DiffuseIBL.SRV.Get(),
		m_EnvironmentTextures.BRDF_Lut.SRV.Get(),
		m_Textures.BaseColor.SRV.Get(),
		m_Textures.Roughness.SRV.Get(),
		m_Textures.Normal.SRV.Get(),
		m_Textures.Metallic.SRV.Get(),

	};

	ID3D11SamplerState* const samplers[] =
	{
		m_SamplerState,
		m_BRDFSampler
	};

	context->PSSetShaderResources ( 0, _countof( srvs ), srvs );
	context->PSSetSamplers ( 0, _countof( samplers ), samplers );
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
	m_World = world;
	m_View = view;
	m_Proj = proj;
}

John::Environment Material::GetEnvironmentTextures() const
{
	return m_EnvironmentTextures;
}

void Material::SetEnvironmentTextures( John::Environment val )
{
	m_EnvironmentTextures = val;
}

ID3D11SamplerState* Material::GetBRDFSampler() const
{
	return m_BRDFSampler;
}

void Material::SetBRDFSampler( ID3D11SamplerState* val )
{
	m_BRDFSampler = val;
}
