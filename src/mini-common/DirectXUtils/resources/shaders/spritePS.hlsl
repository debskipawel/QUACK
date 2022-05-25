Texture2D colorMap : register(t0);
SamplerState texSampler : register(s0);

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 tex :TEXCOORD0;
};

float4 main(PS_INPUT i) : SV_TARGET
{
	return colorMap.Sample(texSampler, i.tex);
}