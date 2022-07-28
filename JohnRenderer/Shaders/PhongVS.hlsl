#include "PhongCommon.hlsli"

cbuffer TransformConstantBuffer : register(b0)
{
	float4x4 MVP;
	float4x4 Model;
}



PSInput main(VSInput input)
{
	PSInput vout;
	vout.PositionPS = mul(input.pos, MVP);
	vout.PositionWS = mul(input.pos, Model);
	vout.Normal = mul(input.norm, Model);
	return vout;

}