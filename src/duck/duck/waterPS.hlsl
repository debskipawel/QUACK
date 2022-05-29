SamplerState samp : register(s0);

cbuffer cbView : register(b0)
{
    matrix viewMatrix;
    matrix invViewMatrix;
};

TextureCube envMap : register(t0);
Texture2D normalMap : register(t1);

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 localPos : POSITION0;
    float3 worldPos : POSITION1;
};

float3 intersectRay(float3 p, float3 r)
{
    float3 t = max((-p + 1) / r, (-p - 1) / r);
    float minT = min(t.x, min(t.y, t.z));

    return p + minT * r;
}

float fresnel(float3 normal, float3 view)
{
    const float F0 = 0.14;
    float cosTetha = max(0.0, dot(normal, view));
	
    return F0 + (1 - F0) * pow(1 - cosTetha, 5);
}

float4 main(PSInput i) : SV_TARGET
{
    float4 camPos = mul(invViewMatrix, float4(0.0, 0.0, 0.0, 1.0));
    
    float3 viewVec = normalize(camPos.xyz - i.worldPos);
    float3 worldNorm = float3(0.0f, 1.0f, 0.0f);

    float2 tex = (i.localPos.xz + 1.0) / 2.0;
    float3 norm = normalMap.Sample(samp, tex);
    norm.x = norm.x * 2.0 - 1.0;
    norm.z = norm.z * 2.0 - 1.0;

    float refractIndex = 0.75;

    if (dot(viewVec, worldNorm) < 0)
    {
        refractIndex = 1.33;
        norm = -norm;
    }

    float3 reflected = reflect(-viewVec, norm);
    float3 refracted = refract(-viewVec, norm, refractIndex);

    float3 reflectedCube = normalize(intersectRay(i.localPos, reflected));
    float3 refractedCube = normalize(intersectRay(i.localPos, refracted));

    float4 reflectedColor = envMap.Sample(samp, reflectedCube);
    float4 refractedColor = envMap.Sample(samp, refractedCube);

    float4 color = reflectedColor;

    float f = fresnel(norm, viewVec);

    if (any(refracted))
    {
        color = f * reflectedColor + (1 - f) * refractedColor;
    }

    return pow(color, 0.4545);
}