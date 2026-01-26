struct PS_IN {
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float4 color : COLOR0;
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
SamplerState samp : register(s0);
float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if(objAmbient.a >= 1.0f)
		color = tex.Sample(samp, pin.uv);
	float3 N = normalize(pin.normal);
	// lightDirはfloat4なので、xyz成分のみを取り出して正規化するのが安全
	float3 L = normalize(-lightDir.xyz);
	float dotNL = saturate((dot(N, L) + 0.7f) / 1.5f);
	float3 diffuse = objDiffuse.rgb * lightDiffuse.rgb;
	float3 ambient = objAmbient.rgb * lightDiffuse.rgb;
	float3 specular = objSpecular.rgb * lightDiffuse.rgb;
	// 本来のLambert拡散反射（思った表現が出来なかったので採用せず
	// color.rgb *= saturate(diffuse * dotNL + ambient);
	// 環境光で拡散反射部分の色が変わらないようにlerp(環境光,diffuse,dotNL)で計算
	// 環境光が弱ければ黒(乗算)、強ければ白(加算)となるように、各計算を線形で補間
	diffuse *= color.rgb;
	color.rgb = saturate(lerp(
		lerp(diffuse * ambient, diffuse + ambient, pow(ambient, 4.0f)),
		diffuse, dotNL));
	// 本来なら必要ない鏡面反射、Lambert向けに若干だけ適用
	color.rgb += specular * pow(saturate(dotNL), max(0.01f, objSpecular.a) * 0.5f) * 0.5f;
	return color;
}