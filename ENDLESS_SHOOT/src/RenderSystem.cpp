#include "RenderSystem.h"
#include "MeshRenderer.h"
#include "SpriteRenderer.h"
#include "Transform.h"
#include "ShaderList.h"
#include "Sprite.h"
#include "CameraSystem.h"
#include "Defines.h"
#include "EffekseerManager.h"
#include <vector>
#include <algorithm>
#include "DirectX.h"

// シャドウマップ解像度
const int SHADOW_MAP_WIDTH = 2048;
const int SHADOW_MAP_HEIGHT = 2048;

// static メンバ変数の実体定義
Microsoft::WRL::ComPtr<ID3D11Texture2D>          RenderSystem::m_pShadowMapTexture;
Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   RenderSystem::m_pShadowMapDSV;
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> RenderSystem::m_pShadowMapSRV;
D3D11_VIEWPORT                                   RenderSystem::m_ShadowViewport = {}; // {} で0初期化
Microsoft::WRL::ComPtr<ID3D11Buffer>             RenderSystem::m_pShadowCB;
static Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pShadowSampler; // サンプラーを保持
Camera RenderSystem::light;

HRESULT RenderSystem::Init()
{
	light.fov = 120.0f;
	light.farClip = 80.0f;
	light.aspect = 1.0f;

	HRESULT hr = S_OK;

	ID3D11Device* pDevice = GetDevice();

	// --------------------------------------------------
	// 1. テクスチャリソース作成
	// --------------------------------------------------
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = SHADOW_MAP_WIDTH;
	texDesc.Height = SHADOW_MAP_HEIGHT;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	// R24G8_TYPELESS を指定 (深度24bit + ステンシル8bit として扱うため)
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	// 深度バッファとして、またシェーダーリソースとして使うフラグ
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	hr = pDevice->CreateTexture2D(&texDesc, nullptr, m_pShadowMapTexture.GetAddressOf());
	if (FAILED(hr)) return hr;

	// --------------------------------------------------
	// 2. Depth Stencil View (DSV) 作成
	//    -> 深度バッファとして使うためのView
	// --------------------------------------------------
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	// 深度フォーマットを指定（ステンシル含む）
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	hr = pDevice->CreateDepthStencilView(m_pShadowMapTexture.Get(), &dsvDesc, m_pShadowMapDSV.GetAddressOf());
	if (FAILED(hr)) return hr;

	// --------------------------------------------------
	// 3. Shader Resource View (SRV) 作成
	//    -> シェーダーでテクスチャとして使うためのView
	// --------------------------------------------------
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	// Rチャンネル(24bit深度部分)を赤色として読み込む
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(m_pShadowMapTexture.Get(), &srvDesc, m_pShadowMapSRV.GetAddressOf());
	if (FAILED(hr)) return hr;

	// --------------------------------------------------
	// 4. ビューポート設定
	//    -> シャドウマップのサイズに合わせた専用Viewportが必要
	// --------------------------------------------------
	m_ShadowViewport.TopLeftX = 0.0f;
	m_ShadowViewport.TopLeftY = 0.0f;
	m_ShadowViewport.Width = static_cast<float>(SHADOW_MAP_WIDTH);
	m_ShadowViewport.Height = static_cast<float>(SHADOW_MAP_HEIGHT);
	m_ShadowViewport.MinDepth = 0.0f;
	m_ShadowViewport.MaxDepth = 1.0f;

	// サンプラーステート作成
	D3D11_SAMPLER_DESC comparisonSamplerDesc;
	ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.BorderColor[0] = 1.0f;
	comparisonSamplerDesc.BorderColor[1] = 1.0f;
	comparisonSamplerDesc.BorderColor[2] = 1.0f;
	comparisonSamplerDesc.BorderColor[3] = 1.0f;
	comparisonSamplerDesc.MinLOD = 0.f;
	comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	comparisonSamplerDesc.MipLODBias = 0.f;
	comparisonSamplerDesc.MaxAnisotropy = 0;
	comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	comparisonSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

	hr = pDevice->CreateSamplerState(
		&comparisonSamplerDesc,
		m_pShadowSampler.GetAddressOf()
	);

	return S_OK;
}

void RenderSystem::Draw(ECS::World* world)
{
	ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
	GetContext()->PSSetShaderResources(
		0,
		2,
		nullSRV
	);

	// シャドウマップへの描画開始
	// レンダーターゲットは設定せず、深度バッファのみセットする (色は書き込まないためnull)
	GetContext()->OMSetRenderTargets(0,nullptr, m_pShadowMapDSV.Get());

	// 深度バッファをクリア
	GetContext()->ClearDepthStencilView(m_pShadowMapDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	// シャドウマップへの描画開始

	// ビューポートをシャドウマップサイズへ変更
	GetContext()->RSSetViewports(1, &m_ShadowViewport);

	SetDepthTest(true);
	SetCullingMode(D3D11_CULL_NONE); // 影の穴あき防止のためNone推奨

	//// ライトのビュー・プロジェクション行列計算
	//DirectX::XMFLOAT4X4 lightView = light.GetViewMatrixLookAt(
	//	Vector3(0.0f, 20.0f, 0.0f), // Eye: ライトの位置
	//	Vector3(0.0f, 0.0f, 0.0f), // Target: オブジェクトがある場所(原点)
	//	Vector3(0.0f, 0.0f, 1.0f)  // Up: カメラの上方向 (真下を見るならZ+を上にすると安定)
	//);
	//DirectX::XMFLOAT4X4 lightProj = light.GetProjectionMatrix();

	// 1. ビュー行列 (LookAtLH)
	DirectX::XMVECTOR eyePos = DirectX::XMVectorSet(0.0f, 20.0f, 0.0f, 1.0f); // ライト位置
	DirectX::XMVECTOR focusPos = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // 注視点
	DirectX::XMVECTOR upDir = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // 上方向
	DirectX::XMMATRIX matLightView = DirectX::XMMatrixLookAtLH(eyePos, focusPos, upDir);

	// 2. プロジェクション行列 (PerspectiveFovLH)
	DirectX::XMMATRIX matLightProj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(120.0f), // FOV
		1.0f,                                // アスペクト比
		0.1f,                                // Near
		80.0f                                // Far
	);

	// 3. シェーダー用に転置 (Transpose) した変数を用意する
	DirectX::XMFLOAT4X4 lightView, lightProj;
	DirectX::XMStoreFloat4x4(&lightView, DirectX::XMMatrixTranspose(matLightView));
	DirectX::XMStoreFloat4x4(&lightProj, DirectX::XMMatrixTranspose(matLightProj));

	// ライトバッファ（メイン描画用）にも転置済みを渡す
	DirectX::XMFLOAT4X4 lightBuffer[2] = { lightView ,lightProj };

	GetContext()->PSSetShader(nullptr, nullptr, 0);
	// シェーダー定数バッファ（ライト視点）を設定して描画
	world->ForEach<MeshRenderer, Transform>([&](ECS::EntityID id, MeshRenderer& mesh, Transform& transform) {
		if (!mesh.isVisible || !mesh.pModel) return;

		// ワールド行列計算
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(transform.position);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(transform.rotation);
		DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(transform.scale);
		DirectX::XMMATRIX matWorld = S * R * T;

		// シェーダー定数バッファ用に行列セット
		DirectX::XMFLOAT4X4 wvp[3];
		DirectX::XMStoreFloat4x4(&wvp[0], DirectX::XMMatrixTranspose(matWorld));
		wvp[1] = lightView;
		wvp[2] = lightProj;

		// シェーダー設定
		ShaderList::SetWVP(wvp);

		for (int i = 0; i < mesh.pModel->GetMeshNum(); ++i) {
			// モデル内のメッシュを描画
			Model::Mesh Mesh = *(mesh.pModel->GetMesh(i));

			// アニメーション用ボーン行列を送信
			if (Mesh.bones.size() > 0)
			{
				DirectX::XMFLOAT4X4 bones[200];
				for (int b = 0; b < 200; ++b) DirectX::XMStoreFloat4x4(&bones[b], DirectX::XMMatrixIdentity());
				for (size_t b = 0; b < Mesh.bones.size() && b < 200; ++b)
				{
					const Model::Bone& bone = Mesh.bones[b];
					DirectX::XMMATRIX mat = bone.invOffset * mesh.pModel->GetBone(bone.index);
					DirectX::XMStoreFloat4x4(&bones[b], DirectX::XMMatrixTranspose(mat));
				}
				ShaderList::SetBones(bones);
				ShaderList::GetVS(ShaderList::VS_SHADOW_ANIME)->Bind();
			}
			else
			{
				ShaderList::GetVS(ShaderList::VS_SHADOW)->Bind();
			}

			//ShaderList::GetPS(ShaderList::PS_SHADOW)->Bind();

			// 描画
			Mesh.pMesh->Draw();
		}
	});

	GetContext()->PSSetShaderResources(
		0,
		2,
		nullSRV
	);

	// 通常描画開始
	// 1. バックバッファ(画面)へ戻す
	auto p = GetDefaultRTV()->GetView();
	GetContext()->OMSetRenderTargets(1, &p, GetDefaultDSV()->GetView()); // ?????DSV

	GetContext()->ClearDepthStencilView(
		GetDefaultDSV()->GetView(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0
	);

	// ビューポートを戻す
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)GetDefaultRTV()->GetWidth();
	vp.Height = (float)GetDefaultRTV()->GetHeight();
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	GetContext()->RSSetViewports(1, &vp);

	// 作成したシャドウマップをテクスチャとしてセット
	ShaderList::SetShadow(m_pShadowMapSRV.Get(), lightBuffer);

	// シャドウマップ用サンプラーをセット
	GetContext()->PSSetSamplers(1, 1, m_pShadowSampler.GetAddressOf());

	DirectX::XMFLOAT4X4 view = CameraSystem::GetView();
	DirectX::XMFLOAT4X4 proj = CameraSystem::GetProjection();
	
	//--- 3Dモデル (MeshRenderer) の描画
	world->ForEach<MeshRenderer, Transform>([&](ECS::EntityID id, MeshRenderer& mesh, Transform& transform) {
		if (!mesh.isVisible || !mesh.pModel) return;

		// ワールド行列計算
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(transform.position);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(transform.rotation);
		DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(transform.scale);
		DirectX::XMMATRIX matWorld = S * R * T;

		// シェーダー定数バッファ用に行列セット
		DirectX::XMFLOAT4X4 wvp[3];
		DirectX::XMStoreFloat4x4(&wvp[0], DirectX::XMMatrixTranspose(matWorld));
		wvp[1] = view;
		wvp[2] = proj;

		// シェーダー設定
		ShaderList::SetWVP(wvp);

		SetDepthTest(true);  // 深度テスト有効化

		for (int i = 0; i < mesh.pModel->GetMeshNum(); ++i) {
			// モデル内のメッシュを描画
			Model::Mesh Mesh = *(mesh.pModel->GetMesh(i));
			// メッシュに割り当てられているマテリアルを取得
			Model::Material	material = *mesh.pModel->GetMaterial(Mesh.materialID);
			// マテリアル情報を調整してからSetMaterialで送信する

			// MeshRendererの色反映
			material.diffuse.x *= mesh.color.x;
			material.diffuse.y *= mesh.color.y;
			material.diffuse.z *= mesh.color.z;
			material.diffuse.w *= mesh.color.w;
			material.ambient.x *= mesh.color.x;
			material.ambient.y *= mesh.color.y;
			material.ambient.z *= mesh.color.z;
			material.ambient.w *= mesh.color.w;

			// シェーダーへマテリアル送信
			ShaderList::SetMaterial(material);

			// アニメーション用ボーン行列を送信
			if (Mesh.bones.size() > 0)
			{
				DirectX::XMFLOAT4X4 bones[200];
				for (int b = 0; b < 200; ++b) DirectX::XMStoreFloat4x4(&bones[b], DirectX::XMMatrixIdentity());
				for (size_t b = 0; b < Mesh.bones.size() && b < 200; ++b)
				{
					const Model::Bone& bone = Mesh.bones[b];
					DirectX::XMMATRIX mat = bone.invOffset * mesh.pModel->GetBone(bone.index);
					DirectX::XMStoreFloat4x4(&bones[b], DirectX::XMMatrixTranspose(mat));
				}
				ShaderList::SetBones(bones);
			}

			// モデルを描画
			mesh.pModel->Draw(i);
		}
	});

	// エフェクトの描画 (3Dモデルの後、2Dスプライトの前)
	EffekseerManager::Draw();

	//--- 2Dスプライト (SpriteRenderer) の描画
	// Spriteクラスにビュー・プロジェクション行列を設定
	Sprite::SetView(view);
	Sprite::SetProjection(proj);

	world->ForEach<SpriteRenderer,Transform>([&](ECS::EntityID id, SpriteRenderer& sprite,Transform& transform) {
		if (!sprite.isVisible || !sprite.pTexture || sprite.isUI) return;

		// ワールド行列計算
		// 3D空間配置のスプライトはTransformのスケールを使う
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(transform.position);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(transform.rotation);
		DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(transform.scale);
		DirectX::XMMATRIX matWorld = S * R * T;

		DirectX::XMFLOAT4X4 worldOut;
		DirectX::XMStoreFloat4x4(&worldOut, DirectX::XMMatrixTranspose(matWorld));

		Sprite::SetWorld(worldOut);
		Sprite::SetTexture(sprite.pTexture.get());
		Sprite::SetColor(DirectX::XMFLOAT4(sprite.color.x, sprite.color.y, sprite.color.z, sprite.color.w));
		Sprite::SetSize(DirectX::XMFLOAT2(1.0f, 1.0f));
		Sprite::SetOffset(DirectX::XMFLOAT2((0.5f - sprite.pivot.x) * sprite.size.x, (0.5f - sprite.pivot.y) * sprite.size.y));
		Sprite::SetUVPos(DirectX::XMFLOAT2(sprite.uvPos.x, sprite.uvPos.y));
		Sprite::SetUVScale(DirectX::XMFLOAT2(sprite.uvScale.x, sprite.uvScale.y));

		Sprite::Draw();
	});

	//--- 2Dスプライト(UI)の描画
	// Spriteクラスにビュー・プロジェクション行列を設定
	// UI用の行列 (正射影)
	DirectX::XMFLOAT4X4 uiView;
	DirectX::XMStoreFloat4x4(&uiView, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));
	DirectX::XMFLOAT4X4 uiProj;
	DirectX::XMStoreFloat4x4(&uiProj, DirectX::XMMatrixTranspose(DirectX::XMMatrixOrthographicOffCenterLH(
		0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f)));

	// UI用設定
	Sprite::SetView(uiView);
	Sprite::SetProjection(uiProj);
	SetDepthTest(false);
	SetCullingMode(D3D11_CULL_NONE);

	struct UIRenderData {
		SpriteRenderer* sprite;
		Transform* transform;
	};
	std::vector<UIRenderData> uiList;

	world->ForEach<SpriteRenderer,Transform>([&](ECS::EntityID id, SpriteRenderer& sprite,Transform& transform) {
		if (!sprite.isVisible || !sprite.pTexture || !sprite.isUI) return;
		uiList.push_back({ &sprite, &transform });
	});

	std::sort(uiList.begin(), uiList.end(), [](const UIRenderData& a, const UIRenderData& b) { return a.sprite->layer < b.sprite->layer; });

	for (const auto& data : uiList) {
		SpriteRenderer& sprite = *data.sprite;
		Transform& transform = *data.transform;

		Vector3 scale = transform.scale;
		scale.y *= -1.0f; // 左上原点座標系にするためY反転

		Vector3 pos = transform.position;
		pos.z = 0.0f; // UIのZ座標は0固定
		
		// ワールド行列計算
		// UI用のスプライトはspriteとtransformのスケールを使う
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(pos);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(transform.rotation);
		DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(scale);
		DirectX::XMMATRIX matWorld = S * R * T;

		DirectX::XMFLOAT4X4 worldOut;
		DirectX::XMStoreFloat4x4(&worldOut, DirectX::XMMatrixTranspose(matWorld));

		Sprite::SetWorld(worldOut);
		Sprite::SetTexture(sprite.pTexture.get());
		Sprite::SetColor(DirectX::XMFLOAT4(sprite.color.x, sprite.color.y, sprite.color.z, sprite.color.w));
		Sprite::SetSize(DirectX::XMFLOAT2(sprite.size.x * sprite.uvScale.x, sprite.size.y * sprite.uvScale.y));
		Sprite::SetOffset(DirectX::XMFLOAT2((0.5f - sprite.pivot.x) * sprite.size.x, (0.5f - sprite.pivot.y) * sprite.size.y));
		Sprite::SetUVPos(DirectX::XMFLOAT2(sprite.uvPos.x, sprite.uvPos.y));
		Sprite::SetUVScale(DirectX::XMFLOAT2(sprite.uvScale.x, sprite.uvScale.y));

		Sprite::Draw();
	}

	// 描画設定をデフォルトに戻す
	SetDepthTest(true);
	SetCullingMode(D3D11_CULL_BACK);

	GetContext()->PSSetShaderResources(
		0,
		2,
		nullSRV
	);
}