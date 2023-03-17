struct PSInput
{
	float4 PositionPS : SV_Position;
	float3 Normal : NORMAL;
};

float4 main(PSInput pin) : SV_TARGET
{
	return float4(0.0f, 1.0f, 0.0f, 1.0f);
}