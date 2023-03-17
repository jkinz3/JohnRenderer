#ifndef __PBRCOMMON_HLSLI__
#define __PBRCOMMON_HLSLI__


static const int MaxPointLights = 5;
struct PointLight
{
	float3 LightPos;
	float pack1;
	float4 LightColor;
	float LightIntensity;
};

cbuffer TransformConstants : register(b0)
{
	float4x4 MVP;
	float4x4 Model;
	float4x4 Normal;
};

cbuffer ShadingConstants : register(b0)
{
	float3 CameraPos;
	float pack1;
	PointLight PointLights[MaxPointLights];
}
	
	struct VSInput
{
	float4 pos : POSITION;
	float3 norm : NORMAL;
	float2 texCoord : TEXCOORD;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct PSInput
{
	float4 PositionPS : SV_Position;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
	float3 PositionWS : TEXCOORD1;
	float3x3 TangentBasis : TBASIS;
};


#endif