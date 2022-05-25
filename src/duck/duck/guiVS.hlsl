cbuffer cbProjection : register(b0)
{
	matrix projMtx;
}

struct VSInput
{
	float2 pos : POSITION0;
	float4 col : COLOR0;
	float2 uv : TEXCOORD0;
};

struct VSOutput
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
	float2 uv : TEXCOORD0;
};

VSOutput main( VSInput i )
{
	VSOutput o;
	o.pos = mul(projMtx, float4(i.pos, 0, 1));
	o.col = i.col;
	o.uv = i.uv;
	return o;
}