#include "PBRCommon.hlsli"

TextureCube CubeMap : register(t0);
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


// ----------------------------------------------------------------------------
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
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;

}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;

}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);


}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	float invRoughness = 1.0 - roughness;
	return F0 + (max(float3(invRoughness.xxx), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);

}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
float3 fresnelSchlick(float3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Returns number of mipmap levels for specular IBL environment map.
uint querySpecularTextureLevels()
{
	uint width, height, levels;
	CubeMap.GetDimensions(0, width, height, levels);
	return levels;
}


float4 main(PSInput pin) : SV_TARGET
{
	float2 TexCoord = pin.TexCoord;
	
	float3 albedo = float3(1.f, 0.f, 0.f); //BaseColorMap.Sample(defaultSampler, TexCoord);
	float roughness = .2f;
	float metalness = 0.f;
	
	float N = normalize(pin.Normal);
	float3 V = normalize(CamPos - pin.PositionWS);
	float3 R = reflect(-V, N);
	
	float3 Lo = float3(0, 0, 0);
	float3 F0 = float3(0.04, .04, .04);
	F0 = lerp(F0, albedo, metalness);
{
	
		//one light source
		float3 L = normalize(LightPos - pin.PositionWS);
		float3 H = normalize(V + L);
		float distance = length(LightPos - pin.PositionWS);
		float attenuation = 1.0 / (distance * distance);
		
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
		
		float3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + .0001;
		
		float3 specular = numerator / denominator;
		
		float3 kS = F;
		float3 kD = float3(1, 1, 1) - kS;
		
		kD *= 1.0 - metalness;
		
		float NdotL = max(dot(N, L), 0.0);
		
		Lo += (kD * albedo / PI + specular) * attenuation * NdotL;
}
	
	Lo = float3(0, 0, 0);
	float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
	float3 kS = F;
	float3 kD = 1.0 - kS;
	kD *= 1.0 - metalness;
	
	float3 irradiance = IrradianceMap.Sample(defaultSampler, N).rgb;
	float3 diffuse = irradiance * albedo;
	
	uint specLevels = querySpecularTextureLevels();
	float3 prefilterColor = CubeMap.SampleLevel(defaultSampler, R, roughness * specLevels).rgb;
	float2 brdf = BRDF_LUT.Sample(BRDF_Sampler, float2(max(dot(N, V), 0.0), roughness)).rg;
	
	float3 specular = prefilterColor * (F * brdf.x + brdf.y);
	
	float3 ambient = (kD * diffuse + specular) * ao;
	float3 color = ambient + Lo;
	
	return float4(color, 1.0);
	
	
	
}