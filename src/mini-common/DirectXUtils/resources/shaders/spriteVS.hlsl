cbuffer vs_spriteTransformBuffer : register(b0)
{
	matrix spriteTransform;
}

cbuffer vs_texTransformBuffer : register(b1)
{
	matrix texTransform;
}


struct VS_INPUT
{
	float2 pos : POSITION0;
	float2 tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 tex :TEXCOORD0;
};

PS_INPUT main(VS_INPUT i)
{
	PS_INPUT o;
	float4 pos = mul(spriteTransform, float4(i.pos, 0.0f, 1.0f));
	o.pos = float4(pos.xy, 0.0f, 1.0f);
	float4 tex = mul(texTransform, float4(i.tex, 0.0f, 1.0f));
	o.tex = tex.xy;
	return o;
}