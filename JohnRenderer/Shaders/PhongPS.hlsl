#include "PhongCommon.hlsli"

cbuffer ShadingConstants : register(b0)
{
	float3 LightPos;
	float pack1;
	float3 CameraPos;
	
}

//Texture2D BrickColor : register(t0);
Texture2D NormalMap : register(t0);
SamplerState standardSampler : register(s0);

float4 main(PSInput pin) : SV_TARGET
{
	float3 objectColor = float3(1, 0, 0); //BrickColor.Sample(standardSampler, pin.TexCoord, 0).rgb;
	float ambientStrength = .1f;
	float3 ambient = objectColor * ambientStrength;
	float2 texCoord = pin.TexCoord * 2.f;
	
	//diffuse 

	float3 N = NormalMap.Sample(standardSampler, pin.TexCoord);
	N = (N * 2.f) - 1.f;
	N = pin.Normal; //normalize(mul(pin.TangentBasis, N));

	
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