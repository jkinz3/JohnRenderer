#include "PhongCommon.hlsli"

cbuffer ShadingConstants : register(b0)
{
	float3 LightPos;
	float pack1;
	float3 CameraPos;
	
}

Texture2D BrickColor : register(t0);
Texture2D NormalMap : register(t1);
TextureCube<float4> EnvMap: register(t2);
SamplerState standardSampler : register(s0);

float4 main(PSInput pin) : SV_TARGET
{
	float3 objectColor = float3(1, 0, 0); //BrickColor.Sample(standardSampler, pin.TexCoord, 0).rgb;
	float ambientStrength = .1f;
	float3 ambient = objectColor * ambientStrength;
	
	//diffuse 

	float3 N = normalize(pin.Normal);
	float3 lightVec = LightPos - pin.PositionWS;
	float3 L = normalize(lightVec);

	float diff = max(dot(L, N), 0);
	float3 diffuse = diff * objectColor;
	
	//specular 
	float specularStrength = .5f;
	float3 V = normalize(CameraPos - pin.PositionWS);
	float3 R = reflect(-L, N);
	float3 H = normalize(L + V);
	float spec = pow(max(dot(H, N), 0.0), 16) * (diff > 0);
	float3 specular = specularStrength * spec;
	
	float distance = length(lightVec);
	float attenuation = 1.0 / (distance * distance);
	
	diffuse *= attenuation;
	specular *= attenuation;
	
	float3 result = (ambient + diffuse + specular);
	
	return float4(result, 1.f);

}