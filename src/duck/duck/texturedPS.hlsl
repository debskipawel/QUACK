Texture2D colorMap : register(t0);
SamplerState colorSampler : register(s0);

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex: TEXCOORD0;
};

float4 main(PSInput i) : SV_TARGET
{
	// TODO : 0.08 Sample the color map at the texture coordinates and return the color. run the program to observe the results
	return float4(0.0f, 0.0f, 0.0f, 1.0f);
}