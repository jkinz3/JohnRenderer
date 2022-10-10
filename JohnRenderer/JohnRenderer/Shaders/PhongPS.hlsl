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
	float3 L = normalize(LightPos - pin.PositionWS);
	float diffuse= max(dot(L, N), 0);
	
	
	float3 result = diffuse * objectColor;
	
	
	return float4(result, 1.f);
	
	////diffuse lighting
	//float3 N = normalize(NormalMap.Sample(standardSampler, pin.TexCoord));
	//N = normalize(mul(pin.TangentBasis, N));
	//float3 L = normalize(LightPos - pin.PositionWS);
	//float diffuseLight = max(dot(L, N), 0);
	//float3 diffuse = diffuseLight;
	
	//float3 View = normalize(pin.PositionWS.xyz - CameraPos);
	//float3 Reflect = reflect(View, normalize(N));
	//float3 Specular = EnvMap.SampleLevel(standardSampler, Reflect, 0).xyz;
	//return float4(objectColor + Specular, 1.f);
	
//	//specular lighting
//	float specularStrength = 1.5f;
//	float3 V = normalize(CameraPos - pin.PositionWS.xyz);
//	float3 R = reflect(-L, N);
//	float3 spec = pow(max(dot(V, R), 0.0), 64);
//	float3 specular = specularStrength * spec;
//	return float4((ambient + diffuse * objectColor) + specular, 1.f); //	+diffuse + specularLight, 1.0f);
}