struct PSInput
{
	float4 PositionPS : SV_Position;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
	float3 PositionWS : TEXCOORD1;
	
};

cbuffer ShadeConstants : register(b0)
{
	float3 CameraPos;
	float pack1;
	float NumPointLights;
}

struct PointLight
{
	float3 LightPos;
	float pack1;
	float4 LightColor;
	float LightIntensity;
};

Texture2D BaseColorMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetallicMap : register(t3);
TextureCube Diff_IBLMap : register(t4);
TextureCube SpecIBLMap : register(t5);
StructuredBuffer<PointLight> PointLights : register(t6);

SamplerState DefaultSampler : register(s0);

static const float PI = 3.1415926535897;
static const float Epsilon = .00001;

static const float dielectric = float3(.04f, .04f, .04f);


float3 FresnelSchlick(float3 F0, float cosTheta)
{
	return F0 + (1.f - F0) * pow(1.f - cosTheta, 5.f);
}

float GaSchlickG1(float cosTheta, float K)
{
	return cosTheta / (cosTheta * (1.f - K) + K);

}

float GaSchlickGGX(float NdotL, float V, float roughness)
{
	float r = roughness + 1.f;
	float k = (r * r) / 8.f;
	return GaSchlickG1(NdotL, k) * GaSchlickG1(V, k);

}

float NdfGGX(float NdotH, float roughness)
{
	float alpha = roughness * roughness;
	float alphasq = alpha * alpha;
	
	float denom = (NdotH * NdotH) * (alphasq - 1.f) + 1.f;
	return alphasq / (PI * denom * denom);

}

float3 TwoChannelNormalX2(float2 normal)
{
	float2 xy = 2.f * normal - 1.f;
	float z = sqrt(1 - dot(xy, xy));
	return float3(xy.x, xy.y, z);
}

float3x3 CalculateTBN(float3 p, float3 N, float2 tex)
{
	float3 dp1 = ddx(p);
	float3 dp2 = ddy(p);
	float2 duv1 = ddx(tex);
	float2 duv2 = ddy(tex);

	float3x3 M = float3x3(dp1, dp2, cross(dp1, dp2));
	float2x3 inverseM = float2x3(cross(M[1], M[2]), cross(M[2], M[0]));
	float3 T = normalize(mul(float2(duv1.x, duv2.x), inverseM));
	float3 B = normalize(mul(float2(duv1.y, duv2.y), inverseM));
	return float3x3(T, B, N);
	
	
	

}

float3 PeturbNormal(float3 localNormal, float3 position, float3 normal, float2 texCoord)
{
	const float3x3 TBN = CalculateTBN(position, normal, texCoord);
	return normalize(mul(localNormal, TBN));
		
}

uint GetSpecularIBLLevels()
{
	uint width, height, levels;
	SpecIBLMap.GetDimensions(0, width, height, levels);
	return levels;

}

float3 SpecularIBL(float3 N, float3 V, float lodBias)
{
	uint levels = GetSpecularIBLLevels();
	float mip = lodBias * levels;
	float3 dir = reflect(-V, N);
	return SpecIBLMap.SampleLevel(DefaultSampler, dir, mip);

}

float3 DiffuseIBL(float3 N)
{
	return Diff_IBLMap.Sample(DefaultSampler, N);

}

float4 main(PSInput pin) : SV_TARGET
{
	float3 BaseColor = BaseColorMap.Sample(DefaultSampler, pin.TexCoord);
	const float Roughness = RoughnessMap.Sample(DefaultSampler, pin.TexCoord);
	float3 localNormal = TwoChannelNormalX2(NormalMap.Sample(DefaultSampler, pin.TexCoord).xy);
	const float Metallic = MetallicMap.Sample(DefaultSampler, pin.TexCoord);
	
	
	float3 DirectLighting = 0.f;
	float3 AmbientLighting = 0.f;
	
	float3 N = PeturbNormal(localNormal, pin.PositionWS, pin.Normal, pin.TexCoord);
	float3 V = normalize(CameraPos - pin.PositionWS);
	
	float NdotV = max(0, dot(N, V));
	
	float3 R = reflect(-V, N);
	
	float3 F0 = lerp(.04, BaseColor, Metallic);
	float3 c_diff = lerp(BaseColor, float3(0, 0, 0), Metallic);
	
	//direct lighting (analytical lights
	{
		for (int i = 0; i < NumPointLights; i++)
		{
			float3 LightPos = PointLights[i].LightPos;
			float3 LightColor = PointLights[i].LightColor;
			float3 LightIntensity = PointLights[i].LightIntensity;

			float3 L = LightPos - pin.PositionWS;
			float distance = length(L);
			L = normalize(L);
			float attenuation = 1.f / (distance * distance);
			float3 radiance = LightColor * attenuation * LightIntensity;
			
			float3 H = normalize(L + V);
			
			float NdotL = max(0.f, dot(N, L));
			float NdotH = max(0.f, dot(N, H));
			float HdotV = max(0.f, dot(H, V));
			
			float3 F = FresnelSchlick(F0, HdotV);
			
			float G = GaSchlickGGX(NdotL, NdotV, Roughness);
			
			float D = NdfGGX(NdotH, Roughness);
			
			
			float3 kD = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), Metallic);
			
			float3 diffuseBRDF = kD * BaseColor;
			
			float3 specularBRDF = (F * D * G) / max(Epsilon, 4.f * NdotL * NdotV);
			
			
			DirectLighting +=(diffuseBRDF + specularBRDF) * radiance * NdotL;


		}
	}
	
	float3 diffuseIBL = DiffuseIBL(N) * c_diff;
	float3 specularIBL = SpecularIBL(N, V, Roughness) * F0;
	
	
	float3 result = DirectLighting + AmbientLighting + diffuseIBL + specularIBL;
	
	return float4(result, 1.f);

}