struct PSInput
{
	float4 PositionPS : SV_Position;
	
};

float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}