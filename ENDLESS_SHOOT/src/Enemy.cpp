#include "Enemy.h"
#include "Collider.h"
#include "Component.h"
#include "Defines.h"
#include "ColliderSystem.h"
#include "Shockwave.h"
#include "CameraSystem.h"
#include "GameManager.h"
#include "PlayerManager.h"

static constexpr float ATTACK_RANGE = 1.8f;
static constexpr float SPEED = 1.0f;
static constexpr float JUMP_POWER = 0.3f;
static constexpr float ATTACK_COOLDOWN = 2.0f;
static constexpr float SPEED_MAX = 8.0f;
static constexpr float HP_OFFSET = 1.8f;
static constexpr float HP_LENGTH = 0.8f;

Enemy::Enemy():
	speed(SPEED),
	damageTimer(0.0f),
	deathSE(INVALID_SOUND_HANDLE)
{
}


void Enemy::OnCreate()
{
	int stage = GameManager::GetStageCount();
	hp = 3 + (stage - 1);
	maxHp = hp;
	currentState = State::CHASE;

	// ステージ数に応じてスピードアップ
	speed = SPEED + (stage - 1) * 0.15f;
	if (speed > SPEED_MAX) speed = SPEED_MAX;

	// HP

	Push([this](ECS::World& world)
		{
	{
		hpBarBgId = world.CreateEntity();
		Transform t;
		world.AddComponent<Transform>(hpBarBgId, t);

		// SpriteRenderer sr;
		// sr.SetTexture("Assets/Texture/white.png");
		// sr.size = Vector2({1.0f, 0.1f});
		// sr.color = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
		// sr.pivot = Vector2({0.5f, 0.5f});
		// sr.isUI = false;
		// world.AddComponent<SpriteRenderer>(hpBarBgId, sr);
	}

	// HP
	{
		hpBarId = world.CreateEntity();
		Transform t;
		t.scale = Vector3(1.0f, 0.1f, 1.0f);
		world.AddComponent<Transform>(hpBarId, t);

		SpriteRenderer sr;
			sr.SetTexture("Assets/Texture/white.png");
		sr.size = Vector2({1.0f, 1.0f});
		sr.color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		sr.pivot = Vector2({0.0f, 0.5f});
		sr.isUI = false;
		world.AddComponent<SpriteRenderer>(hpBarId, sr);
	}});

	// 死亡音ロード
	deathSE = LoadSound("Assets/Sound/enemyDeath/DSGNImpt_EXPLOSION-Forced Shutdown_HY_PC-001.wav");
	SetVolume(deathSE, 0.8f);
}

void Enemy::Update()
{
	Vector3 targetPos;
	bool found = false;
	ECS::EntityID playerId = -1;

	// ワールド内の全てのエンティティからプレイヤー(タグがPLAYER)を探す
	world->ForEach<BoxCollider, Transform>([&](ECS::EntityID id, BoxCollider& col, Transform& trans) {
		if (col.tag == CollisionTag::PLAYER)
		{
			targetPos = trans.position;
			playerId = id;
			found = true;
		}
	});

	// プレイヤーが見つかったら追跡する
	if (found)
	{
		auto& myTrans = world->GetComponent<Transform>(entityId);

		// プレイヤーへの方向ベクトルを計算
		Vector3 dir = targetPos - myTrans.position;
		float distance = dir.Magnitude();
		float dt = 1.0f / fFPS; // デルタタイム

		switch (currentState)
		{
		case State::CHASE:
			// 攻撃範囲に入ったらATTACKステートへ移行
			if (distance <= ATTACK_RANGE)
			{
				currentState = State::ATTACK;
				// ジャンプする
				GetComponent<Rigidbody>().velocity.y += JUMP_POWER;
			}
			else
			{
				// プレイヤーに向かって移動 (速度 * デルタタイム)
				dir.y = 0;
				Vector3 moveDir = dir.Normalized();

				// 障害物回避処理 (レイキャスト)
				ColliderSystem::Ray ray;
				ray.origin = myTrans.position + Vector3(0.0f, 0.5f, 0.0f); // 足元より少し上から
				ray.direction = moveDir;
				ColliderSystem::RaycastHit hit;
				float checkDist = 2.0f; // 検知距離

				// 正面に障害物があるかチェック (自分自身は無視)
				if (ColliderSystem::Instance().Raycast(world, ray, &hit, checkDist, entityId))
				{
					// プレイヤー以外なら回避行動をとる
					if (hit.hitObj != playerId)
					{
						// 左45度をチェック
						Vector3 leftDir = Quaternion::FromRotation(0.0f, -45.0f, 0.0f) * moveDir;
						ray.direction = leftDir;
						bool hitLeft = ColliderSystem::Instance().Raycast(world, ray, &hit, checkDist, entityId);
						if (hitLeft && hit.hitObj == playerId) hitLeft = false; // プレイヤーならOK

						// 右45度をチェック
						Vector3 rightDir = Quaternion::FromRotation(0.0f, 45.0f, 0.0f) * moveDir;
						ray.direction = rightDir;
						bool hitRight = ColliderSystem::Instance().Raycast(world, ray, &hit, checkDist, entityId);
						if (hitRight && hit.hitObj == playerId) hitRight = false; // プレイヤーならOK

						if (!hitLeft) moveDir = leftDir;      // 左が空いていれば左へ
						else if (!hitRight) moveDir = rightDir; // 右が空いていれば右へ
						else moveDir = Quaternion::FromRotation(0.0f, -90.0f, 0.0f) * moveDir; // どちらもダメなら大きく左へ
					}
				}

				// 進行方向に向く
				if (moveDir.MagnitudeSq() > 0.0001f)
				{
					float radian = atan2(moveDir.x, moveDir.z);
					float degree = DirectX::XMConvertToDegrees(radian);
					myTrans.rotation = Quaternion::FromRotation(0.0f, degree, 0.0f);
				}

				myTrans.position += moveDir * speed * dt;
			}
			break;

		case State::ATTACK:

			if (GetComponent<Rigidbody>().velocity.y > 0.0f)
				break;
			// 着地時
		{
			ColliderSystem::Ray ray;
			ray.origin = myTrans.position;
			ray.origin.y += 1.0f;
			ray.direction = { 0.0f, -1.0f, 0.0f };
			ColliderSystem::RaycastHit hit;

			// 自分自身(entityId)を無視してレイキャストを行う
			if (ColliderSystem::Instance().Raycast(world, ray, &hit, 2.0f, entityId))
			{
				// 攻撃判定を出す
				Transform t = myTrans;
				Push([t](ECS::World& w) {
					ECS::EntityID id = w.CreateEntity();

					// Transform
					Transform transform = t;
					transform.scale = Vector3(1.0f, 1.0f, 1.0f);
					w.AddComponent<Transform>(id, transform);

					// Collider
					BoxCollider bc;
					bc.size = Vector3(3.0f, 1.0f, 3.0f);
					bc.isTrigger = true;
					bc.tag = CollisionTag::ENEMY;
					w.AddComponent<BoxCollider>(id, bc);

					// Script
					Script script;
					script.Bind<Shockwave>();
					w.AddComponent<Script>(id, script);
				});

				currentState = State::WAIT;
				waitTimer = 0.0f;
			}
		}

			break;

		case State::WAIT:

			waitTimer += 1.0f / fFPS;

			if (waitTimer >= ATTACK_COOLDOWN)
			{
				currentState = State::CHASE;
			}
			break;
		}
	}

	// HPバー Update
	if (world->HasComponent<Transform>(hpBarBgId) && world->HasComponent<Transform>(hpBarId))
	{
		auto& myTrans = GetComponent<Transform>();
		Vector3 barPos = myTrans.position + Vector3(0.0f, HP_OFFSET, 0.0f);

		// ビルボード
		Quaternion q = Quaternion::Identity();
		ECS::EntityID camId = CameraSystem::GetCamera();
		if (world->HasComponent<Transform>(camId)) {
			q = world->GetComponent<Transform>(camId).rotation;
		}

		// BG
		auto& bgT = world->GetComponent<Transform>(hpBarBgId);
		bgT.position = barPos;
		bgT.rotation = q;

		// FG
		auto& fgT = world->GetComponent<Transform>(hpBarId);
		Vector3 right = q * Vector3(HP_LENGTH, 0.0f, 0.0f);
		fgT.position = barPos - (right * 0.5f);
		fgT.rotation = q;

		float ratio = (float)hp / (float)maxHp;
		if (ratio < 0.0f) ratio = 0.0f;
		fgT.scale.x = ratio * HP_LENGTH;
	}

	// ダメージ演出 (赤く点滅)
	if (damageTimer > 0.0f)
	{
		damageTimer -= 1.0f / fFPS;
		if (world->HasComponent<MeshRenderer>(entityId))
		{
			auto& mr = world->GetComponent<MeshRenderer>(entityId);
			// 点滅処理 (一定間隔で赤と白を切り替え)
			int flash = (int)(damageTimer * 15.0f) % 2;
			if (flash == 0) mr.color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
			else mr.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
	else
	{
		if (world->HasComponent<MeshRenderer>(entityId))
		{
			world->GetComponent<MeshRenderer>(entityId).color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
}

void Enemy::OnCollisionEnter(ECS::EntityID other)
{
	auto& collider = world->GetComponent<BoxCollider>(other);
		
	// 弾と当たったらダメージ
	if (collider.tag == CollisionTag::BULLET)
	{
		hp -= PlayerManager::GetAttackPower();
		damageTimer = 0.5f; // 0.5秒間点滅

		if (hp <= 0)
		{
			Play(deathSE);
			ECS::EntityID id = this->entityId;
			Push([id](ECS::World& w) {
				w.DeleteEntity(id);
			});
		}
	}
}

Enemy::~Enemy()
{
	// シーン遷移時(World破棄時)にPushするとCommandBufferが既に破棄されている可能性があるため、
	// 撃破時(HP<=0)のみ削除コマンドを発行する
	if (hp <= 0) {
		if (hpBarBgId != -1) Push([id = hpBarBgId](ECS::World& w) { w.DeleteEntity(id); });
		if (hpBarId != -1) Push([id = hpBarId](ECS::World& w) { w.DeleteEntity(id); });
	}
}