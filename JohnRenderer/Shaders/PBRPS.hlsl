#include "PBRCommon.hlsli"

TextureCube SpecularTexture : register(t0);
TextureCube IrradianceMap : register(t1);
Texture2D BRDF_LUT: register(t2);

Texture2D BaseColorMap : register(t3);
Texture2D RoughnessMap : register(t4);
Texture2D NormalMap : register(t5);
//Texture2D MetallicMap : register(t6);

SamplerState defaultSampler : register(s0);
SamplerState BRDF_Sampler : register(s1);

static const float ao = 1.f;
static const float FDielectric = .04f;

static const float PI = 3.1415926535897f;
static const float Epsilon = .00001;

static const float3 Fdielectirc = float3(.04, .04, .04);

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float NdfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}
// Single term for separable Schlick-GGX below.
float GaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float GaSchlickGGX(float cosLi, float V, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return GaSchlickG1(cosLi, k) * GaSchlickG1(V, k);
}

// Shlick's approximation of the Fresnel factor.
float3 FresnelSchlick(float3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
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
	uint width, height, levels;
	BaseColorMap.GetDimensions(0, width, height, levels);

	
	const float2 texCoord = pin.TexCoord * 1.f;
	const float Roughness = RoughnessMap.Sample(defaultSampler, texCoord).r;
	const float Metalness = 0.f; //MetallicMap.Sample(defaultSampler, texCoord).r;
	const float3 BaseColor = BaseColorMap.Sample(defaultSampler, texCoord).rgb;

	float3 worldNormal = normalize(pin.Normal);
	float2 tangentNormal = NormalMap.Sample(defaultSampler, texCoord).rg;
	float2 xy = 2.f * tangentNormal - 1.f;
	float z = sqrt(1 - dot(xy, xy));
	float3 localNormal = float3(xy.x, xy.y, z);
	
	float3 N = normalize(mul(localNormal, pin.TangentBasis));
	float3 V = normalize(CameraPos - pin.PositionWS);
	
	float cosLo = max(0, dot(N, V));
	
	float3 Lr = 2.0 * cosLo * N - V;
	
	float3 F0 = lerp(Fdielectirc, BaseColor, Metalness);
	
	float3 directLighting = 0.0;
	{
		for (uint i = 0; i < MaxPointLights; ++i)
		{
			PointLight Light = PointLights[i];
			float3 LightColor = float3(1, 1, 1);
			float3 Li = normalize(Light.LightPos - pin.PositionWS);
			float distance = length(Light.LightPos - pin.PositionWS);
			float attenuation = 1.0 / (distance * distance);
			float3 Lradiance = LightColor * attenuation;
			

			float3 Lh = normalize(Li + V);
			
			float cosLi = max(0.0, dot(N, Li));
			float cosLh = max(0.0, dot(N, Lh));
			
			float3 F = FresnelSchlick(F0, max(0.0, dot(Lh, V)));
			
			float D = NdfGGX(cosLh, Roughness);
			
			float G = GaSchlickGGX(cosLi, cosLo, Roughness);
			
			float3 kD = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), Metalness);
			
			float3 diffuseBRDF = kD * BaseColor;
			
			float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);
			
			directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;

		}
		

	}
	
	float3 ambientLighting = 0.f;
		
		float3 irradiance = IrradianceMap.Sample(defaultSampler, N).rgb;
			
		float3 F = FresnelSchlick(F0, cosLo);
			
		float3 kD = lerp(1.0 - F, 0.0, Metalness);
			
		float3 diffuseIBL = kD * BaseColor * irradiance;
			
		uint specularTextureLevels = QuerySpecularTextureLevels();
		float lod = Roughness * specularTextureLevels;
		float3 specularIrradiance = SpecularTexture.SampleLevel(defaultSampler, Lr, Roughness * specularTextureLevels ).rgb;
			
		float2 specularBRDF = BRDF_LUT.Sample(BRDF_Sampler, float2(cosLo, Roughness)).rg;
			
		float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
		ambientLighting = diffuseIBL + specularIBL;


	
	return float4(ambientLighting + directLighting, 1.0);

}