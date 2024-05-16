struct Vertex
{
	float4 pos : POSITION;
};

struct PSInput
{
	float4 PositionPS : SV_Position;
	
};

cbuffer MVPConstant : register(b0)
{
	float4x4 MVP;
}

PSInput main(Vertex vertex)
{
	PSInput vout;
	vout.PositionPS = mul(vertex.pos, MVP);
	
	return vout;

}