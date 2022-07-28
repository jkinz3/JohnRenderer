#ifndef __PHONGCOMMON_HLSLI__
#define __PHONGCOMMON_HLSLI__



struct VSInput
{
	float4 pos : POSITION;
	float3 norm : NORMAL;
	float2 texCoord : TEXCOORD;
};

struct PSInput
{
	float4 PositionPS : SV_Position;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
	float3 PositionWS : TEXCOORD1;
};

#endif