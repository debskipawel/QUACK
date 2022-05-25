sampler fontSampler : register(s0);
texture2D fontTexture : register(t0);

struct PSInput
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
	float2 uv : TEXCOORD0;
};

float4 main(PSInput i) : SV_TARGET
{
	return saturate(i.col * fontTexture.Sample(fontSampler, i.uv));
}