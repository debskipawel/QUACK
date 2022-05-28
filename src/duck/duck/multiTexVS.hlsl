cbuffer cbWorld : register(b0) //Vertex Shader constant buffer slot 0
{
	matrix worldMatrix;
};

cbuffer cbView : register(b1) //Vertex Shader constant buffer slot 1
{
	matrix viewMatrix;
};

cbuffer cbProj : register(b2) //Vertex Shader constant buffer slot 2
{
	matrix projMatrix;
};

cbuffer cbTexture1Transform : register(b3)
{
	matrix texMatrix1;
};

cbuffer cbTexture2Transform : register(b4)
{
	matrix texMatrix2;
};

struct VSInput
{
	float3 pos : POSITION;
	float3 norm : NORMAL0;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex1: TEXCOORD0;
	float2 tex2: TEXCOORD1;
};

PSInput main(VSInput i)
{
	PSInput o = (PSInput)0;
	o.pos = float4(i.pos, 1.0f);

	// TODO : 1.01 Calculate coordinates for each of two textures

	o.pos = mul(worldMatrix, o.pos);
	o.pos = mul(viewMatrix, o.pos);
	o.pos = mul(projMatrix, o.pos);

	return o;
}