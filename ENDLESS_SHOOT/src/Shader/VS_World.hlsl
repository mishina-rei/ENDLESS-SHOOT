struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};
struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float4 wPos : POSITION0;
    float4 lightSpacePos : POSITION1; // ★ライト空間での座標
};
cbuffer WVP : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
};
cbuffer LightBuffer : register(b1)
{
    float4x4 LightView;
    float4x4 LightProjection;
};
VS_OUT main(VS_IN vin)
{
    VS_OUT vout;
    vout.pos = float4(vin.pos, 1.0f);
    vout.pos = mul(vout.pos, world);
    vout.wPos = vout.pos;
    vout.pos = mul(vout.pos, view);
    vout.pos = mul(vout.pos, proj);
    vout.normal = mul(vin.normal, (float3x3) world);
    vout.uv = vin.uv;
    vout.color = vin.color;
    //現在の頂点が「ライトから見たらどこにあるか」を計算
    vout.lightSpacePos = mul(vout.wPos, LightView);
    vout.lightSpacePos = mul(vout.lightSpacePos, LightProjection);
    return vout;
}