struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};

cbuffer Material : register(b0)
{
    float4 objDiffuse;
    float4 objAmbient;
    float4 objSpecular;
};

cbuffer Light : register(b1)
{
    float4 lightDiffuse;
    float4 lightDir;
};

Texture2D tex : register(t0);
Texture2D shadowMap : register(t1);
SamplerState samp : register(s0);
SamplerComparisonState shadowSamp : register(s1); // シャドウマップ専用サンプラー


float4 main(PS_IN pin) : SV_TARGET
{
    // そのまま色を表示
    float4 color = tex.Sample(samp, pin.uv);
    return color;
}
