TextureCube colorMap : register(t0);
SamplerState colorSampler : register(s0);

cbuffer cbSurfaceColor : register(b0)
{
	float4 surfaceColor;
}

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 tex: TEXCOORD0;
};

float4 main(PSInput i) : SV_TARGET
{
	// TODO : 1.24 Sample the color map and multiply it by the surface color
	return surfaceColor;
}