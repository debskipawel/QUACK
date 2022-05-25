#define NLIGHTS 2

float4 lightPos[NLIGHTS];
float3 lightColor[NLIGHTS];
float3 surfaceColor;
float ks, kd, ka, m;

float4 phong(float3 worldPos, float3 norm, float3 view)
{
	view = normalize(view);
	norm = normalize(norm);
	float3 color = surfaceColor * ka; //ambient
	for (int k = 0; k < NLIGHTS; ++k)
	{
		float3 lightVec = normalize(lightPos[k].xyz - worldPos);
		float3 halfVec = normalize(view + lightVec);
		color += lightColor[k] * kd * surfaceColor * saturate(dot(norm, lightVec));//diffuse
		color += lightColor[k] * ks * pow(saturate(dot(norm, halfVec)), m);//specular
	}
	return saturate(float4(color, 1.0f));
}

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 worldPos : POSITION0;
	float3 norm : NORMAL0;
	float3 view : VIEWVEC0;
};

float4 main(PSInput i) : SV_TARGET
{
	return phong(i.worldPos, i.norm, i.view);
}