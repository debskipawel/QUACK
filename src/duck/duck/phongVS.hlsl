cbuffer cbWorld : register(b0) //Vertex Shader constant buffer slot 0
{
	matrix worldMatrix;
};

cbuffer cbView : register(b1) //Vertex Shader constant buffer slot 1
{
	matrix viewMatrix;
	matrix invViewMatrix;
};

cbuffer cbProj : register(b2) //Vertex Shader constant buffer slot 2
{
	matrix projMatrix;
};

struct VSInput
{
	float3 pos : POSITION;
	float3 norm : NORMAL0;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 worldPos : POSITION0;
	float3 norm : NORMAL0;
	float3 viewVec : TEXCOORD0;
};

PSInput main(VSInput i)
{
	PSInput o;
	o.worldPos = mul(worldMatrix, float4(i.pos, 1.0f)).xyz;
	o.pos = mul(viewMatrix, float4(o.worldPos, 1.0f));
	o.pos = mul(projMatrix, o.pos);
	o.norm = mul(worldMatrix, float4(i.norm, 0.0f)).xyz;
	o.norm = normalize(o.norm);
	float3 camPos = mul(invViewMatrix, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
	o.viewVec = camPos - o.worldPos;
	return o;
}