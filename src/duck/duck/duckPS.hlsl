SamplerState samp : register(s0);
Texture2D tex : register(t0);
Texture2D noise : register(t1);

cbuffer cbLights : register(b0)
{
    float4 lightPos[2];
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 tex : TEXTURE0;
    float3 worldPos : POSITION0;
    float3 norm : NORMAL0;
    float3 viewVec : TEXCOORD0;
};

static const float3 ambientColor = float3(0.2f, 0.2f, 0.2f);
static const float3 lightColor = float3(1.0f, 1.0f, 1.0f);
static const float kd = 0.5, ks = 0.2f, m = 100.0f;

float4 main(PSInput i) : SV_TARGET
{
    float4 surfaceColor = tex.Sample(samp, i.tex);
    
    float3 viewVec = normalize(i.viewVec);
    float3 normal = normalize(i.norm);
    float3 color = surfaceColor.rgb * ambientColor;
    
    // specular multiplier calculated from noise
    float g = 0.0, l = 0.0;
    const float N = 16;
    
    float2 texPos = (i.tex + 1.0) / 2.0;
    
    // i am not responsible for this abomination, this guy is: https://www.shadertoy.com/view/tldfD8
    for (int j = 0; j < N; j++)
    {
        g += (-0.5 + noise.Sample(samp, texPos * float2(0.06, 4.18) + j * float2(7.0 + 1.0 / N, 7.0 + 1.0 / N) / 256.0).x);
        l += (-0.5 + noise.Sample(samp, float2(0.0, 0.5) + texPos * float2(0.04, 0.004) + j * float2(10.0 + 1.0 / N, 10.0 + 1.0 / N) / 256.0).x);
    }
    
    g *= sqrt(1.0 / N);
    l *= sqrt(1.0 / N);
    
    l = exp(4.0 * l - 1.5);
    g = exp(1.2 * g - 1.5);
    
    float v = 0.1 * g + 0.2 * l + 2.0 * g * l;
    float multiplier = pow(v, 0.4545);
    
    // phong shading
    for (int k = 0; k < 1; k++)
    {
        float3 lightPosition = lightPos[k].xyz;
        float3 lightVec = normalize(lightPosition - i.worldPos);
        float3 halfVec = normalize(viewVec + lightVec);
        
        color += multiplier * lightColor * surfaceColor.xyz * kd * saturate(dot(normal, lightVec)); //diffuse color
        
        float nh = dot(normal, halfVec);
        nh = saturate(nh);
        nh = pow(nh, m);
        nh *= ks;
        color += multiplier * lightColor * nh; // specular
    }
    
    return float4(saturate(color), surfaceColor.a);
}