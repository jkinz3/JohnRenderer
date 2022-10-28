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
	
	float3 T = normalize(mul(input.tangent, (float3x3)Model));
	float3 B = normalize(mul(input.bitangent, (float3x3)Model));
	float3 N = normalize(mul(input.norm, (float3x3)Model));
	vout.TangentBasis = float3x3(T, B, N);
	

	
	return vout;

}