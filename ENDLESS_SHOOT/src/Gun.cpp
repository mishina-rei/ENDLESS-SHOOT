#include "Gun.h"
#include "Component.h"
#include "ShaderList.h"
#include "Defines.h"
#include "Input.h"
#include "Bullet.h"
#include "ColliderSystem.h"
#include "EntityTag.h"
#include "Audio.h"
#include "Random.h"
#include "PlayerManager.h"

Gun::Gun(ECS::EntityID _playerId):
	cameraId(_playerId),
	offset(),
	shootSE(INVALID_SOUND_HANDLE),
	reloadSE(INVALID_SOUND_HANDLE),
	hitSE(INVALID_SOUND_HANDLE),
	magazineSize(PlayerManager::GetMagazineSize()),
	currentAmmo(PlayerManager::GetMagazineSize()),
	reloadTime(2.0f),
	isReloading(false),
	reloadTimer(0.0f),
	ammoBarId(0),
	fireRate(PlayerManager::GetFireRate()),
	fireTimer(0.0f),
	currentRecoil(0.0f, 0.0f, 0.0f),
	hitMarkerId(0),
	hitMarkerAlpha(0.0f)
{
	// プレイヤーから銃へのオフセットを設定
	offset = Vector3(0.6f, -0.6f, 0.0f);
	shootSE = LoadSound("Assets/Sound/shoot/556 Single Isolated WAV.wav");
	SetVolume(shootSE, 0.5f);
	// リロード音
	reloadSE = LoadSound("Assets/Sound/reload/22LR Bolt Mag Reload Full WAV.wav");
	SetVolume(reloadSE, 0.6f);
	// ヒット音
	hitSE = LoadSound("Assets/Sound/hit/DSGNImpt_EXPLOSION-Pyro Burst_HY_PC-005.wav");
}

void Gun::OnCreate()
{
	// 銃の初期化処理
	ECS::EntityID ID = this->entityId;
	Push([this, ID](ECS::World& world)
		{
			// Transform付ける
			Transform transform;
			transform.scale = Vector3(0.5f, 0.5f, 0.5f);
			world.AddComponent<Transform>(ID, transform);

			// MeshRendererつける
			MeshRenderer mr;
			mr.pModel = std::make_shared<Model>();
			mr.pModel->Load("Assets/Model/gun/sniper_0.fbx");
			mr.pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
			mr.pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
			world.AddComponent<MeshRenderer>(ID, mr);

			// レティクル
			{
				ECS::EntityID reticleId = world.CreateEntity();
				Transform reticleTransform;
				reticleTransform.position = Vector3((float)SCREEN_WIDTH * 0.5f, (float)SCREEN_HEIGHT * 0.5f, 0.0f);
				world.AddComponent<Transform>(reticleId, reticleTransform);

				SpriteRenderer sr;
				sr.SetTexture("Assets/Texture/re.png");
				sr.size = {5.0f,5.0f};
				sr.color = {0.0f,0.0f,0.0f,0.7f};
				sr.isUI = true;
				world.AddComponent<SpriteRenderer>(reticleId, sr);
			}

			// 残弾バーUI (背景)
			{
				ECS::EntityID bgId = world.CreateEntity();
				Transform t;
				t.position = Vector3((float)SCREEN_WIDTH - 220.0f, (float)SCREEN_HEIGHT - 40.0f, 0.0f);
				world.AddComponent<Transform>(bgId, t);

				SpriteRenderer sr;
				sr.SetTexture("Assets/Texture/white.png");
				sr.size = Vector2{ 200.0f, 20.0f };
				sr.color = Vector4(0.2f, 0.2f, 0.2f, 1.0f); // 暗いグレー
				sr.pivot = Vector2{ 0.0f, 0.0f };
				sr.isUI = true;
				sr.layer = SpriteLayer::Default;
				world.AddComponent<SpriteRenderer>(bgId, sr);
			}
			// 残弾バーUI (前面・黄色)
			{
				this->ammoBarId = world.CreateEntity();
				Transform t;
				t.position = Vector3((float)SCREEN_WIDTH - 220.0f, (float)SCREEN_HEIGHT - 40.0f, 0.0f);
				world.AddComponent<Transform>(this->ammoBarId, t);

				SpriteRenderer sr;
				sr.SetTexture("Assets/Texture/white.png");
				sr.size = Vector2{ 200.0f, 20.0f };
				sr.color = Vector4(1.0f, 1.0f, 0.0f, 1.0f); // 黄色
				sr.pivot = Vector2{ 0.0f, 0.0f };
				sr.isUI = true;
				sr.layer = SpriteLayer::UI_Front;
				world.AddComponent<SpriteRenderer>(this->ammoBarId, sr);
			}

			// ヒットマーカーUI
			{
				this->hitMarkerId = world.CreateEntity();
				Transform t;
				t.position = Vector3((float)SCREEN_WIDTH * 0.5f, (float)SCREEN_HEIGHT * 0.5f, 0.0f);
				t.scale = Vector3(0.6f, 0.6f, 1.0f);
				world.AddComponent<Transform>(this->hitMarkerId, t);

				SpriteRenderer sr;
				sr.SetTexture("Assets/Texture/hitmark.png"); // ヒットマーカー画像
				sr.color = Vector4(1.0f, 1.0f, 1.0f, 0.0f); // 最初は透明
				sr.isUI = true;
				sr.layer = SpriteLayer::UI_Front;
				world.AddComponent<SpriteRenderer>(this->hitMarkerId, sr);
			}
		});
}

void Gun::Update()
{
	// ヒットマーカーのフェードアウト処理
	if (hitMarkerAlpha > 0.0f)
	{
		hitMarkerAlpha -= 2.0f * (1.0f / fFPS); // フェードアウト速度
		if (hitMarkerAlpha < 0.0f) hitMarkerAlpha = 0.0f;

		if (hitMarkerId != 0 && world->HasComponent<SpriteRenderer>(hitMarkerId)) {
			world->GetComponent<SpriteRenderer>(hitMarkerId).color.w = hitMarkerAlpha;
		}
	}

	// 反動の減衰 (徐々に0に戻す)
	currentRecoil = currentRecoil * 0.85f;

	// リロード中であればタイマーを進める
	if (isReloading)
	{
		reloadTimer -= 1.0f / fFPS;
		if (reloadTimer <= 0.0f)
		{
			// リロード完了
			isReloading = false;
			currentAmmo = magazineSize;
		}
		return; // リロード中は他の操作を受け付けない
	}

	// 発射タイマー更新
	if (fireTimer > 0.0f)
	{
		fireTimer -= 1.0f / fFPS;
	}

	// 左クリック長押しで発射
	if (IsKeyPress(VK_LBUTTON) && fireTimer <= 0.0f)
	{
		Shoot();
		fireTimer = fireRate;
	}

	// 'R'キーでリロード
	if (IsKeyTrigger('R'))
	{
		// 弾が満タンでなく、リロード中でないならリロードする
		if (currentAmmo < magazineSize && !isReloading)
		{
			Reload();
		}
	}

	// 弾がないときは発射ボタンでリロード
	if (IsKeyTrigger(VK_LBUTTON))
	{
		// 弾が満タンでなく、リロード中でないならリロードする
		if (currentAmmo <= 0 && !isReloading)
		{
			Reload();
		}
	}

	// 残弾バーの更新
	if (ammoBarId != 0 && world->HasComponent<Transform>(ammoBarId))
	{
		auto& t = world->GetComponent<Transform>(ammoBarId);
		float ratio = (float)currentAmmo / (float)magazineSize;
		if (ratio < 0.0f) ratio = 0.0f;
		t.scale.x = ratio;
	}
}

void Gun::LateUpdate()
{
	// プレイヤーのTransformコンポーネントを持っているか確認
	if (world->HasComponent<Transform>(cameraId))
	{
		const auto& playerTransform = world->GetComponent<Transform>(cameraId);
		auto& myTransform = GetComponent<Transform>();

		// プレイヤーの回転に合わせてオフセットを回転させる
		Vector3 rotatedOffset = playerTransform.rotation * (offset + currentRecoil);

		// 座標と回転を更新
		myTransform.position = playerTransform.position + rotatedOffset;
		myTransform.rotation = playerTransform.rotation;
	}
}

void Gun::OnDestroy()
{
	if (hitMarkerId != 0) {
		ECS::EntityID id = hitMarkerId;
		Push([id](ECS::World& w) { w.DeleteEntity(id); });
	}
}

void Gun::Shoot()
{
	// 弾切れ、またはリロード中なら撃てない
	if (currentAmmo <= 0 || isReloading)
	{
		// TODO: 弾切れの音を鳴らすなど
		return;
	}

	currentAmmo--;

	// 発砲処理
	Play(shootSE);

	// 反動を加える (手前に引く + ランダムなブレ)
	currentRecoil.z -= 0.2f; // 手前(Zマイナス)にキックバック
	currentRecoil.x += Random::Range(-0.05f, 0.05f); // 横ブレ
	currentRecoil.y += Random::Range(-0.05f, 0.05f); // 縦ブレ

	// 弾の向き計算
	Transform myTrans = GetComponent<Transform>();
	Transform cameraTrans = world->GetComponent<Transform>(cameraId);

	// 目標地点
	Vector3 targetPos;

	// プレイヤーのIDを取得してRaycastで無視する
	ECS::EntityID playerId = -1;
	world->ForEachComponent<PlayerTag>([&](ECS::EntityID id, PlayerTag& tag) {
		playerId = id;
	});

	ColliderSystem::Ray ray;
	ray.origin = cameraTrans.position;
	ray.direction = cameraTrans.rotation * Vector3(0, 0, 1);
	ColliderSystem::RaycastHit hit;
	if (ColliderSystem::Instance().Raycast(world, ray, &hit, 100.0f, playerId)) {
		targetPos = hit.point;
	}
	else {
		targetPos = ray.origin + ray.direction * 100.0f;
	}
		
	// 弾の発射処理
	Push([myTrans,targetPos, this](ECS::World& world) {
		ECS::EntityID id = world.CreateEntity();

		// Transform (銃の位置・回転をコピー)
		Transform t;
		t.position = myTrans.position;
		t.rotation = myTrans.rotation;
		t.scale = Vector3(0.1f, 0.1f, 0.1f);
		world.AddComponent<Transform>(id, t);

		// MeshRenderer (弾のモデル)
		MeshRenderer mr;
		mr.pModel = std::make_shared<Model>();
		// ※適切な弾のモデルファイルがあればパスを変更してください
		mr.pModel->Load("Assets/Model/bullet.fbx");
		mr.pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		mr.pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		world.AddComponent<MeshRenderer>(id, mr);

		// Rigidbody (移動用)
		Rigidbody rb;
		rb.useGravity = false; // 重力無効
		Vector3 forward = Vector3(0, 0, 1);
		forward = t.rotation * forward; // 向いている方向へ
		rb.velocity = forward * 1.0f;   // 弾速
		Vector3 dir = (targetPos - t.position).Normalized();
		rb.velocity = dir * 1.0f;   // 弾速
		world.AddComponent<Rigidbody>(id, rb);

		// BoxCollider (当たり判定)
		BoxCollider bc;
		bc.size = Vector3(0.1f, 0.1f, 0.1f);
		bc.tag = CollisionTag::BULLET;
		world.AddComponent<BoxCollider>(id, bc);

		// Script (寿命管理など)
		Script script;
		script.Bind<Bullet>(this->entityId); // 銃のIDを渡す
		world.AddComponent<Script>(id, script);
		});

	// 弾を撃ち切ったら自動でリロードする
	if (currentAmmo <= 0)
	{
		Reload();
	}
}

void Gun::Reload()
{
	if (isReloading) return;

	isReloading = true;
	reloadTimer = reloadTime;
	Play(reloadSE);
}

void Gun::ShowHitMarker()
{
	hitMarkerAlpha = 1.0f;
	if (hitMarkerId != 0 && world->HasComponent<SpriteRenderer>(hitMarkerId)) {
		world->GetComponent<SpriteRenderer>(hitMarkerId).color.w = hitMarkerAlpha;
	}

	// ヒット音再生
	Play(hitSE);
}
