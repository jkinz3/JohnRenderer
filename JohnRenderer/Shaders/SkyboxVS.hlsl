#include "SkyboxCommon.hlsli"



PSInput main( VSInput vin)
{
	PSInput vout;
	vout.PositionPS = mul(vin.Position, MVP);
	vout.PositionPS.z = vout.PositionPS.w;
	vout.TexCoord = vin.Position.xyz;

	return vout;
}