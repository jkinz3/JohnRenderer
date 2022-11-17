#include "PBRCommon.hlsli"

TextureCube SpecularTexture : register(t0);
TextureCube IrradianceMap : register(t1);
Texture2D BRDF_LUT: register(t2);

//Texture2D BaseColorMap : register(t3);
//Texture2D RoughnessMap : register(t4);
Texture2D NormalMap : register(t5);
//Texture2D MetallicMap : register(t6);

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
	
	return nom /denom;

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
float3 FresnelSchlick(float3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);

}
// ----------------------------------------------------------------------------
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
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
	const float Roughness = .2f; //RoughnessMap.Sample(defaultSampler, texCoord).r;
	const float Metalness = 0.f; //MetallicMap.Sample(defaultSampler, texCoord).r;
	const float3 BaseColor = float3(1.f, 0.f, 0.f); //BaseColorMap.Sample(defaultSampler, texCoord);
	float3 N = normalize(2.0 * NormalMap.Sample(defaultSampler, texCoord).rgb - 1.0);
	N = normalize(mul(pin.TangentBasis, N));
	N = pin.Normal;

	float3 V = normalize(CameraPos - pin.PositionWS);
	float3 R = reflect(-V, N);
	
	float3 F0 = float3(0.04, .04, .04);
	F0 = lerp(F0, BaseColor, Metalness);
	
	
	//direct light for analytical lights
	float3 directLighting = 0.0;
	{
		for (int i = 0; i < 1; ++i)
		{
			float3 L = normalize(LightPos - pin.PositionWS);
			float3 H = normalize(V + L);
			float distance = length(LightPos - pin.PositionWS);
			float attenuation = 1.0 / (distance * distance);
			float3 radiance = float3(1.f, 1.f, 1.f) * attenuation;
			
			//cook-torrance brdf
			float NDF = DistributionGGX(N, H, Roughness);
			float G = GeometrySmith(N, V, L, Roughness);
			float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

			float3 Numerator = NDF * G * F;
			float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + .0001;
			float3 specular = Numerator / denominator;
			
			float3 kS = F;
			
			float3 kD = float3(1.0, 1.0, 1.0) - kS;
			
			kD *= 1.0 - Metalness;
			
			float NdotL = max(dot(N, L), 0.0);
			
			directLighting += (kD * BaseColor / PI + specular) * radiance * NdotL;

		}

	}
	
	//ambient lighting from image
	float3 ambientLighting = float3(.3f, 0.f, 0.f);
	{

	}
	
	return float4(directLighting + ambientLighting, 1.0);

}