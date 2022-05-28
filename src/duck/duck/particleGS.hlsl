cbuffer cbProj : register(b0) //Geometry Shader constant buffer slot 0
{
	matrix projMatrix;
};

struct GSInput
{
	float4 pos : POSITION;
	float age : TEXCOORD0;
	float angle : TEXCOORD1;
	float size : TEXCOORD2;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex1: TEXCOORD0;
	float2 tex2: TEXCOORD1;
};

static const float TimeToLive = 4.0f;

[maxvertexcount(4)]
void main(point GSInput inArray[1], inout TriangleStream<PSInput> ostream)
{
	GSInput i = inArray[0];
	float sina, cosa;
	sincos(i.angle, sina, cosa);
	float dx = (cosa - sina) * 0.5 * i.size;
	float dy = (cosa + sina) * 0.5 * i.size;
	PSInput o = (PSInput)0;

	// TODO : 1.30 Initialize 4 vertices to make a bilboard and append them to the ostream

	ostream.RestartStrip();
}