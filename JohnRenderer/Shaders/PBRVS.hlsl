#include "PBRCommon.hlsli"



PSInput main(VSInput vin)
{
	PSInput vout;
	
	vout.PositionPS = mul(vin.pos, MVP);
	vout.PositionWS = mul(vin.pos, Model).xyz;
	
	vout.TexCoord = vin.texCoord;
	
	float4 norm = float4(vin.norm, 1.0);
	norm = mul(norm, Model);
	vout.Normal = normalize(norm.xyz);
	
	float3x3 TBN = float3x3(vin.tangent, vin.bitangent, vin.norm);
	vout.TangentBasis = mul((float3x3) Model, transpose(TBN));
	
	return vout;
	
	

}