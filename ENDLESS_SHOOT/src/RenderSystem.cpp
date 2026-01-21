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

void RenderSystem::Draw(ECS::World* world)
{
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