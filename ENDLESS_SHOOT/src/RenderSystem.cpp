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

// シャドウマップの解像度
const int SHADOW_MAP_WIDTH = 2048;
const int SHADOW_MAP_HEIGHT = 2048;

// static メンバ変数の実体を定義（メモリを確保）します
Microsoft::WRL::ComPtr<ID3D11Texture2D>          RenderSystem::m_pShadowMapTexture;
Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   RenderSystem::m_pShadowMapDSV;
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> RenderSystem::m_pShadowMapSRV;
D3D11_VIEWPORT                                   RenderSystem::m_ShadowViewport = {}; // {} で0初期化しておくと安全です
Camera RenderSystem::light;

HRESULT RenderSystem::Init()
{
	light.farClip = 100.0f;

	HRESULT hr = S_OK;

	ID3D11Device* pDevice = GetDevice();

	// --------------------------------------------------
	// 1. テクスチャリソースの作成
	// --------------------------------------------------
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = SHADOW_MAP_WIDTH;
	texDesc.Height = SHADOW_MAP_HEIGHT;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	// ★重要: R24G8_TYPELESS を指定 (深度24bit + ステンシル8bit の器を作る)
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	// ★重要: 深度バッファとしても、シェーダーリソースとしても使うフラグ
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	hr = pDevice->CreateTexture2D(&texDesc, nullptr, m_pShadowMapTexture.GetAddressOf());
	if (FAILED(hr)) return hr;

	// --------------------------------------------------
	// 2. Depth Stencil View (DSV) の作成
	//    -> 実際に深度を書き込むためのView
	// --------------------------------------------------
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Flags = 0;
	// ここで深度フォーマットとして解釈させる
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	hr = pDevice->CreateDepthStencilView(m_pShadowMapTexture.Get(), &dsvDesc, m_pShadowMapDSV.GetAddressOf());
	if (FAILED(hr)) return hr;

	// --------------------------------------------------
	// 3. Shader Resource View (SRV) の作成
	//    -> シェーダーでテクスチャとして読むためのView
	// --------------------------------------------------
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	// Rチャンネル(24bit深度部分)を赤色成分として読む設定
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	hr = pDevice->CreateShaderResourceView(m_pShadowMapTexture.Get(), &srvDesc, m_pShadowMapSRV.GetAddressOf());
	if (FAILED(hr)) return hr;

	// --------------------------------------------------
	// 4. ビューポートの設定
	//    -> シャドウマップのサイズに合わせた専用のViewportが必要
	// --------------------------------------------------
	m_ShadowViewport.TopLeftX = 0.0f;
	m_ShadowViewport.TopLeftY = 0.0f;
	m_ShadowViewport.Width = static_cast<float>(SHADOW_MAP_WIDTH);
	m_ShadowViewport.Height = static_cast<float>(SHADOW_MAP_HEIGHT);
	m_ShadowViewport.MinDepth = 0.0f;
	m_ShadowViewport.MaxDepth = 1.0f;

	return S_OK;
}

void RenderSystem::Draw(ECS::World* world)
{
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	GetContext()->PSSetShaderResources(
		0,
		1,
		nullSRV
	);
	// シャドウマップへの書き込み
	// レンダーターゲットを解除し、深度バッファだけセットする (色は書き込まないためnull)
	ID3D11RenderTargetView* nullRTV = nullptr;
	GetContext()->OMSetRenderTargets(1, &nullRTV, m_pShadowMapDSV.Get());

	// 深度バッファをクリア
	GetContext()->ClearDepthStencilView(m_pShadowMapDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// ビューポートをシャドウマップサイズに変更
	GetContext()->RSSetViewports(1, &m_ShadowViewport);

	// シェーダーや定数バッファを「ライト視点」に設定して描画
	world->ForEach<MeshRenderer, Transform>([&](ECS::EntityID id, MeshRenderer& mesh, Transform& transform) {
		if (!mesh.isVisible || !mesh.pModel) return;

		// ワールド行列の計算
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(transform.position);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(transform.rotation);
		DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(transform.scale);
		DirectX::XMMATRIX matWorld = S * R * T;

		// シェーダー定数バッファ用の行列セット
		DirectX::XMFLOAT4X4 wvp[3];
		DirectX::XMStoreFloat4x4(&wvp[0], DirectX::XMMatrixTranspose(matWorld));
		wvp[1] = light.GetViewMatrix(Vector3(0.0f,10.0f,0.0f),Quaternion(0.0f,0.0f,0.0f));
		wvp[2] = light.GetProjectionMatrix();

		// シェーダーに設定
		ShaderList::SetWVP(wvp);

		//Model* pModel = mesh.pModel;

		SetDepthTest(true);  // 深度テスト有効化

		for (int i = 0; i < mesh.pModel->GetMeshNum(); ++i) {
			//モデルのメッシュを取得
			Model::Mesh Mesh = *(mesh.pModel->GetMesh(i));
			//メッシュに割り当てられているマテリアルを取得
			Model::Material	material = *mesh.pModel->GetMaterial(Mesh.materialID);
			//マテリアルを編集する場合、SetMaterial関数へ設定する前に変更 

			// MeshRendererの色を反映
			material.diffuse.x *= mesh.color.x;
			material.diffuse.y *= mesh.color.y;
			material.diffuse.z *= mesh.color.z;
			material.diffuse.w *= mesh.color.w;
			material.ambient.x *= mesh.color.x;
			material.ambient.y *= mesh.color.y;
			material.ambient.z *= mesh.color.z;
			material.ambient.w *= mesh.color.w;

			//シェーダーへマテリアルを設定
			ShaderList::SetMaterial(material);

			// アニメーション用ボーン行列の送信
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

			//モデルの描画
			mesh.pModel->Draw(i);
		}
		});
	
	// 通常の描画処理へ戻す
	// 1. バックバッファ(画面)に戻す
	auto p = GetDefaultRTV()->GetView();
	GetContext()->OMSetRenderTargets(1, &p, GetDefaultDSV()->GetView()); // 通常のDSV

	// ビューポートも戻す
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)GetDefaultRTV()->GetWidth();
	vp.Height = (float)GetDefaultRTV()->GetHeight();
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	GetContext()->RSSetViewports(1, &vp);

	// 4. さっき作ったシャドウマップをテクスチャとしてセット (t0 レジスタ)
	GetContext()->PSSetShaderResources(0, 1, m_pShadowMapSRV.GetAddressOf());

	DirectX::XMFLOAT4X4 view = CameraSystem::GetView();
	DirectX::XMFLOAT4X4 proj = CameraSystem::GetProjection();
	
	//--- 3Dモデル (MeshRenderer) の描画
	world->ForEach<MeshRenderer, Transform>([&](ECS::EntityID id, MeshRenderer& mesh, Transform& transform) {
		if (!mesh.isVisible || !mesh.pModel) return;

		// ワールド行列の計算
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(transform.position);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(transform.rotation);
		DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(transform.scale);
		DirectX::XMMATRIX matWorld = S * R * T;

		// シェーダー定数バッファ用の行列セット
		DirectX::XMFLOAT4X4 wvp[3];
		DirectX::XMStoreFloat4x4(&wvp[0], DirectX::XMMatrixTranspose(matWorld));
		wvp[1] = view;
		wvp[2] = proj;

		// シェーダーに設定
		ShaderList::SetWVP(wvp);

		//Model* pModel = mesh.pModel;

		SetDepthTest(true);  // 深度テスト有効化

		for (int i = 0; i < mesh.pModel->GetMeshNum(); ++i) {
			//モデルのメッシュを取得
			Model::Mesh Mesh = *(mesh.pModel->GetMesh(i));
			//メッシュに割り当てられているマテリアルを取得
			Model::Material	material = *mesh.pModel->GetMaterial(Mesh.materialID);
			//マテリアルを編集する場合、SetMaterial関数へ設定する前に変更 
			//material.ambient.x = 0.85f; // xは赤(r)を示す 
			//material.ambient.y = 0.85f; // yは緑(g)を示す 
			//material.ambient.z = 0.85f; // zは青(b)を示す

			// MeshRendererの色を反映
			material.diffuse.x *= mesh.color.x;
			material.diffuse.y *= mesh.color.y;
			material.diffuse.z *= mesh.color.z;
			material.diffuse.w *= mesh.color.w;
			material.ambient.x *= mesh.color.x;
			material.ambient.y *= mesh.color.y;
			material.ambient.z *= mesh.color.z;
			material.ambient.w *= mesh.color.w;

			//シェーダーへマテリアルを設定
			ShaderList::SetMaterial(material);

			// アニメーション用ボーン行列の送信
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

			//モデルの描画
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

		// ワールド行列の計算
		// 3D空間に配置するスプライトはTransformのスケールを使う
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

	//--- 2Dスプライト(UI)のみ描画
	// Spriteクラスにビュー・プロジェクション行列を設定
	// UI用の行列作成 (左上原点)
	DirectX::XMFLOAT4X4 uiView;
	DirectX::XMStoreFloat4x4(&uiView, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));
	DirectX::XMFLOAT4X4 uiProj;
	DirectX::XMStoreFloat4x4(&uiProj, DirectX::XMMatrixTranspose(DirectX::XMMatrixOrthographicOffCenterLH(
		0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f)));

	// UI用の設定
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
		scale.y *= -1.0f; // 左上原点座標系に合わせてY軸を反転

		Vector3 pos = transform.position;
		pos.z = 0.0f; // UIはZ座標を0に固定
		
		// ワールド行列の計算
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
}