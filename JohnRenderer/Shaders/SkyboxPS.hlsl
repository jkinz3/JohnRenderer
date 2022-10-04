#include "SkyboxCommon.hlsli"

TextureCube<float4> CubeMap : register(t0);
TextureCube<float4> IrrMap: register(t1);
SamplerState Sampler : register(s0);

float4 main(PSInput pin) : SV_Target0
{
	float3 envColor = CubeMap.SampleLevel(Sampler, normalize(pin.TexCoord), 0);
	
	return float4(envColor, 1.0);

}