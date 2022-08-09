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
	vout.Normal = mul(input.norm, (float3x3)Model);
	vout.TexCoord = input.texCoord;
	float3x3 TBN = float3x3(input.tangent, input.bitangent, input.norm);
	vout.TangentBasis = mul((float3x3) Model, transpose(TBN));
	
	return vout;

}