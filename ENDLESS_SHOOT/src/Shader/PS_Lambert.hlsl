struct PS_IN {
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float4 color : COLOR0;
    float4 wPos : POSITION0; // 頂点シェーダーでワールド座標を計算して渡す
    float4 lightSpacePos : POSITION1; // ライト空間での座標
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

// 影判定関数
float CalculateShadow(float4 lightSpacePos)
{
    // 透視投影の割り算
    // 範囲が [-1, 1] になる
    float3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

    // UV座標系 [0, 1] に変換
    // (-1 -> 0, 1 -> 1) になるように補正
    projCoords.x = projCoords.x * 0.5 + 0.5;
    projCoords.y = -projCoords.y * 0.5 + 0.5; // DirectXはテクスチャ座標のYが下向きなので反転

    // 範囲外（ライトの後ろや範囲外）は影にしない
    if (projCoords.z > 1.0 || projCoords.z < 0.0)
        return 1.0;
    
    // 今描画しようとしているピクセルの深度
    float currentDepth = projCoords.z;

    // シャドウバイアス
    float bias = 0.005;

    // 記録された深度より奥にあれば「影」
    // (1.0 = 明るい, 0.0 = 影)
    return shadowMap.SampleCmpLevelZero(shadowSamp, projCoords.xy, currentDepth - bias);
}

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if(objAmbient.a >= 1.0f)
		color = tex.Sample(samp, pin.uv);
	color = float4(color.rgb,1.0f);
	float3 texColor = color.rgb; // 影の計算用にテクスチャカラーを保存
    float3 N = normalize(pin.normal);
	// lightDirはfloat4なので、xyz成分のみを取り出して正規化するのが安全
    float3 L = normalize(lightDir.xyz);
    float dotNL = saturate(-dot(N, L));
    float3 diffuse = objDiffuse.rgb * lightDiffuse.rgb ;
    float3 ambient = objAmbient.rgb * lightDiffuse.rgb + float3(0.2f, 0.2f, 0.2f);  // 暗すぎるから足す
    float3 specular = objSpecular.rgb * lightDiffuse.rgb;
    
	// 本来のLambert拡散反射（思った表現が出来なかったので採用せず
    color.rgb *= saturate(diffuse * dotNL + ambient);
    
	// 本来なら必要ない鏡面反射、Lambert向けに若干だけ適用
    color.rgb += specular * pow(saturate(dotNL), max(0.01f, objSpecular.a) * 0.5f) * 0.5f;
	
	// 影の適用
    float shadowFactor = CalculateShadow(pin.lightSpacePos);
    
    // 影弱くする
    shadowFactor = 1.0f - (1.0f - shadowFactor) * 0.5f;
    
    // shadowFactorが0(影)なら環境光(ambient)のみ、1(光)なら元の計算結果を適用
    color.rgb = lerp(ambient * texColor, color.rgb, shadowFactor);
    
	return color;
}
