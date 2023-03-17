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
};


cbuffer TransformConstants : register(b0)
{
	float4x4 MVP;
	float4x4 Model;
};

PSInput main(VSInput vin)
{
	PSInput vout;
	vout.PositionPS = mul(vin.pos, MVP);
	vout.Normal = mul(vin.norm, (float3x3) Model);
	
	return vout;
}