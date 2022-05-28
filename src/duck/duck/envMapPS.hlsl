TextureCube colorMap : register(t0);
SamplerState colorSampler : register(s0);

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 tex: TEXCOORD0;
};

float4 main(PSInput i) : SV_TARGET
{
    float4 color = colorMap.Sample(colorSampler, i.tex);
    return pow(color, 0.4545);
}