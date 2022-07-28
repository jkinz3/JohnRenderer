#include "PhongCommon.hlsli"

cbuffer ShadingConstants : register(b0)
{
	float3 LightPos;
	float pack1;
	float3 CameraPos;
	
}

float4 main(PSInput pin) : SV_TARGET
{
	float3 objectColor = float3(1.f, 0.f, 0.f);
	float ambientStrength = .2f;
	float3 ambient = ambientStrength * objectColor;
	float3 N = normalize(pin.Normal);
	float3 L = normalize(LightPos - pin.PositionWS.xyz);
	float NdotL = max(dot(L, N), 0);

	float3 diffuse = ambient + NdotL * objectColor;
	
	return float4(diffuse, 1.0f);
}