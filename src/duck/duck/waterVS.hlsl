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

struct VSOutput
{
    float4 pos : SV_POSITION;
    float3 localPos : POSITION0;
    float3 worldPos : POSITION1;
};

VSOutput main(float3 pos : POSITION0)
{
    VSOutput o = (VSOutput) 0;

    o.localPos = pos / 10;

    o.worldPos = mul(worldMatrix, float4(pos, 1.0));

    o.pos = mul(projMatrix, mul(viewMatrix, float4(o.worldPos, 1.0)));

    return o;
}
