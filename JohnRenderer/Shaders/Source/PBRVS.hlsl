struct Vertex
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

cbuffer TransformConstants : register(b0)
{
	float4x4 MVP;
	float4x4 Model;
}

PSInput main(Vertex vertex)
{
	PSInput vout;
	vout.PositionPS = mul(vertex.pos, MVP);
	vout.Normal = mul(vertex.norm, (float3x3) Model);
	vout.TexCoord = vertex.texCoord;
	vout.PositionWS = mul(vertex.pos, Model);
	
	return vout;

}