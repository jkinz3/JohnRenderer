#ifndef __SKYBOXCOMMON_HLSLI__
#define __SKYBOXCOMMON_HLSLI__


cbuffer SkyConstants : register(b0)
{
	float4x4 MVP;
	float4x4 Model;
};

struct VSInput
{
	float4 Position : Position;
	float3 TexCoord : TEXCOORD0;
};

struct PSInput
{
	float4 PositionPS : SV_Position;
	float3 TexCoord : TEXCOORD0;
};
	
#endif