#include "PBRCommon.hlsli"



PSInput main(VSInput vin)
{
	PSInput vout;
	vout.PositionPS = mul(vin.pos, MVP);
	vout.PositionWS = mul(vin.pos, Model);
	vout.Normal = mul(vin.norm, (float3x3) Model);
	vout.TexCoord = vin.texCoord;
	
	float3 T = normalize(mul(vin.tangent, (float3x3) Model));
	float3 B = normalize(mul(vin.bitangent, (float3x3) Model));
	float3 N = normalize(mul(vin.norm, (float3x3) Model));
	vout.TangentBasis = float3x3(T, B, N);
	

	
	return vout;

	

}