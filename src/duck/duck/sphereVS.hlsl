matrix modelMtx, modelInvTMtx, viewProjMtx;
float4 camPos;

struct VSInput
{
	float3 pos : POSITION0;
	float3 norm : NORMAL0;
};

struct VSOutput
{
	float4 pos : SV_POSITION;
	float3 worldPos : POSITION0;
	float3 norm : NORMAL0;
	float3 view : VIEWVEC0;
};

VSOutput main( VSInput i)
{
	VSOutput o;
	float4 worldPos = mul(modelMtx, float4(i.pos, 1.0f));
	o.view = normalize(camPos.xyz - worldPos.xyz);
	o.norm = normalize(mul(modelInvTMtx, float4(i.norm, 0.0f)).xyz);
	o.worldPos = worldPos.xyz;
	o.pos = mul(viewProjMtx, worldPos);
	return o;
}