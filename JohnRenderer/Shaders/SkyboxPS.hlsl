TextureCube<float4> CubeMap : register(t0);
SamplerState Sampler : register(s0);

float4 main(float3 texCoord : TEXCOORD0) : SV_TARGET0
{
	return CubeMap.SampleLevel(Sampler, normalize(texCoord), 0);
}