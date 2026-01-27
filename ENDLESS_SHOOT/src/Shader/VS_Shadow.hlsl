cbuffer LightBuffer : register(b0)
{
    matrix LightViewProjection; // ライトの View * Projection
    matrix World;
};

float4 main(float4 pos : POSITION) : SV_POSITION
{
    // モデルを「ライトから見た空間」へ変換
    float4 worldPos = mul(pos, World);
    return mul(worldPos, LightViewProjection);
}