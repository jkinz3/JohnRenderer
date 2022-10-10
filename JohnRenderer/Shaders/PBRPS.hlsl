#include "PBRCommon.hlsli"

TextureCube SpecularTexture : register(t0);
TextureCube IrradianceMap : register(t1);
Texture2D BRDF_LUT: register(t2);

Texture2D BaseColorMap : register(t3);
Texture2D RoughnessMap : register(t4);
Texture2D NormalMap : register(t5);
Texture2D MetallicMap : register(t6);

SamplerState defaultSampler : register(s0);
SamplerState BRDF_Sampler : register(s1);

static const float ao = 1.f;
static const float FDielectric = .04f;

static const float PI = 3.1415926535897f;
static const float Epsilon = .00001;


float DistributionGGX(float3 N, float3 H, float roughness)
{
	
	return 0.f;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
	return 0.f;

}
// ----------------------------------------------------------------------------
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	return 0.f;

}
// ----------------------------------------------------------------------------
float3 fresnelSchlick(float3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);

}
// ----------------------------------------------------------------------------
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	return 0.f;

}

// Returns number of mipmap levels for specular IBL environment map.
uint QuerySpecularTextureLevels()
{
	uint width, height, levels;
	SpecularTexture.GetDimensions(0, width, height, levels);
	return levels;
}

float4 main(PSInput pin) : SV_TARGET
{

	
	const float2 texCoord = pin.TexCoord * 1.f;
	const float Roughness = RoughnessMap.Sample(defaultSampler, texCoord).r;
	const float Metalness = MetallicMap.Sample(defaultSampler, texCoord).r;
	const float3 BaseColor = BaseColorMap.Sample(defaultSampler, texCoord);
	float3 N = normalize(2.0 * NormalMap.Sample(defaultSampler, texCoord).rgb - 1.0);
	N = normalize(mul(pin.TangentBasis, N));
	
	//direction from fragment to eye
	float3 Lo = normalize(CamPos - pin.PositionWS);
	
	float cosLo = max(0.0, dot(N, Lo));
	
	float3 R = 2.0 * cosLo * N - Lo;
	
	//fresnel reflectance at normal, incidence
	float3 F0 = lerp(FDielectric, BaseColor, Metalness);
	
	//direct light for analytical lights
	float3 directLighting = 0.0;
	{
		//not yet actually
	}
	
	//ambient lighting from image
	float3 ambientLighting;
	{
		float3 irradiance = IrradianceMap.Sample(defaultSampler, N).rgb;
		
		//calculate fresnel term for ambient lighting
		float3 F = fresnelSchlick(F0, cosLo);
		
		//get diffuse constribution factor
		float3 kd = lerp(1.0 - F, 0.0, Metalness);
		
		float3 diffuseIBL = kd * BaseColor * irradiance;
		
		//sample pre-filtered specular env at correct mip level
		uint specLevels = QuerySpecularTextureLevels();
		float3 specularIrradiance = SpecularTexture.SampleLevel(defaultSampler, R, Roughness * specLevels).rgb;
		
		//split sum for cook-torrance specular brdf
		float2 specularBRDF = BRDF_LUT.Sample(BRDF_Sampler, float2(cosLo, Roughness)).rg;
		
		//total specular IBL contribution
		float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
		
		//total ambient light
		ambientLighting = diffuseIBL + specularIBL;

	}
	
	return float4(directLighting + ambientLighting, 1.0);

}