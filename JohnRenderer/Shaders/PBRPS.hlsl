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
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	float invRoughness = max(1.0 - roughness, 0.0);
	float invCosTheta = clamp(1.0 - cosTheta, 0.0, 1.0);
	return F0 + (max(invRoughness.xxx, F0) - F0) * pow(invCosTheta.xxx, 5.0);

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
	float3 albedo = float3(1.0, 0.0, 0.0);
	float metalness = 0.f;
	float roughness = .2f;
	float3 N = normalize(pin.Normal); //normalize(2.0 * NormalMap.Sample(defaultSampler, pin.TexCoord).rgb - 1.0);
//	N = normalize(mul(pin.TangentBasis, N));
	float3 V = normalize(CamPos - pin.PositionWS);
	float3 R = reflect(-V, N);
	
	float3 F0 = float3(.04, .04, .04);
	F0 = lerp(F0, albedo, metalness);
	
	float3 Lo = float3(0, 0, 0);
	{
		//todo: multiple lights
	}
	
	float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
	
	float3 kS = F;
	float3 kD = 1.0 - kS;
	kD *= 1.0 - metalness;
	
	float3 irradiance = IrradianceMap.Sample(defaultSampler, N).rgb;
	float3 diffuse = irradiance * albedo;
	
	uint specularLevels = QuerySpecularTextureLevels();
	float3 prefilterColor = SpecularTexture.SampleLevel(defaultSampler, R, roughness * specularLevels).rgb;
	float2 brdf = BRDF_LUT.Sample(BRDF_Sampler, float2(max(dot(N, V), 0.0), roughness)).rg;
	float3 specular = prefilterColor * (F * brdf.x + brdf.y);
	
	float3 ambient = (kD * diffuse + specular) * ao;
	
	float3 color = ambient + Lo;
	
	return float4(prefilterColor, 1.0);
	
}