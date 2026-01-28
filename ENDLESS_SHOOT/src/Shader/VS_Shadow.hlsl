cbuffer WVP : register(b0)
{
    matrix World;
    matrix View;
    matrix Proj;
};

float4 main(float3 pos : POSITION) : SV_POSITION
{
    float4 p = float4(pos, 1.0f);
    p = mul(p, World);
    p = mul(p, View);
    p = mul(p, Proj);
    return p;
}