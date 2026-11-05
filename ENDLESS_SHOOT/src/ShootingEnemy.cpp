#include "ShootingEnemy.h"
#include "Collider.h"
#include "Component.h"
#include "Defines.h"
#include "ColliderSystem.h"
#include "EnemyBullet.h"
#include "CameraSystem.h"
#include "EntityTag.h"
#include "ShaderList.h"
#include "GameManager.h"
#include "PlayerManager.h"

static constexpr float SPEED = 0.8f; // 通常の敵より少し遅くする
static constexpr float HP_OFFSET = 1.8f;
static constexpr float HP_LENGTH = 0.8f;

ShootingEnemy::ShootingEnemy() :
	speed(SPEED),
	damageTimer(0.0f),
	moveTimer(0.0f),
	deathSE(INVALID_SOUND_HANDLE)
{
}

void ShootingEnemy::OnCreate()
{
	int stage = GameManager::GetStageCount();
	hp = 3 + (stage - 1);
	maxHp = hp;
	currentState = State::CHASE;

	// ステージ数に応じてスピードアップ (上限2.0f)
	speed = SPEED + (stage - 1) * 0.08f;
	if (speed > 2.0f) speed = 2.0f;

	// HP
	Push([this](ECS::World& world)
	{
		hpBarId = world.CreateEntity();
		Transform t;
		t.scale = Vector3(1.0f, 0.1f, 1.0f);
		world.AddComponent<Transform>(hpBarId, t);

		SpriteRenderer sr;
		sr.SetTexture("Assets/Texture/white.png");
		sr.size = Vector2{ 1.0f, 1.0f };
		sr.color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		sr.pivot = Vector2{ 0.0f, 0.5f };
		sr.isUI = false;
		world.AddComponent<SpriteRenderer>(hpBarId, sr);
	});


	GetComponent<MeshRenderer>().pModel->Play(0, true);

	// 死亡音ロード
	deathSE = LoadSound("Assets/Sound/enemyDeath/DSGNImpt_EXPLOSION-Forced Shutdown_HY_PC-001.wav");
	SetVolume(deathSE, 0.8f);
}

void ShootingEnemy::Update()
{
	Vector3 targetPos;
	bool found = false;
	ECS::EntityID playerId = -1;

	// プレイヤーを探す
	world->ForEach<BoxCollider, Transform>([&](ECS::EntityID id, BoxCollider& col, Transform& trans) {
		if (col.tag == CollisionTag::PLAYER)
		{
			targetPos = trans.position;
			playerId = id;
			found = true;
		}
	});

	if (found)
	{
		auto& myTrans = world->GetComponent<Transform>(entityId);
		Vector3 dir = targetPos - myTrans.position;
		float distance = dir.Magnitude();
		float dt = 1.0f / fFPS;

		// 常にプレイヤーの方を向く
		dir.y = 0;
		if (dir.MagnitudeSq() > 0.0001f)
		{
			Vector3 lookDir = dir.Normalized();
			float radian = atan2(lookDir.x, lookDir.z);
			float degree = DirectX::XMConvertToDegrees(radian);
			myTrans.rotation = Quaternion::FromRotation(0.0f, degree, 0.0f);
		}

		switch (currentState)
		{
		case State::CHASE:
			// 射程に入ったら攻撃モードへ
			if (distance <= attackRange)
			{
				currentState = State::ATTACK;
			}
			else
			{
				// 移動処理（障害物回避含む）
				Vector3 moveDir = dir.Normalized();
				
				// 簡易的な障害物回避
				ColliderSystem::Ray ray;
				ray.origin = myTrans.position + Vector3(0.0f, 0.5f, 0.0f);
				ray.direction = moveDir;
				ColliderSystem::RaycastHit hit;
				if (ColliderSystem::Instance().Raycast(world, ray, &hit, 2.0f, entityId))
				{
					if (hit.hitObj != playerId)
					{
						// 障害物があれば少し避ける
						moveDir = Quaternion::FromRotation(0.0f, 45.0f, 0.0f) * moveDir;
					}
				}

				myTrans.position += moveDir * speed * dt;
			}
			break;

		case State::ATTACK:
			// プレイヤーが離れすぎたら追いかける
			if (distance > attackRange * 1.2f)
			{
				currentState = State::CHASE;
			}
			else
			{
				// 攻撃中も動きをつける
				moveTimer += dt;

				if (distance < attackRange * 0.4f)
				{
					// プレイヤーが近すぎる場合は後退する
					Vector3 backDir = (myTrans.position - targetPos).Normalized();
					backDir.y = 0;
					myTrans.position += backDir * speed * dt;
				}
				else
				{
					// 程よい距離なら左右に揺れる (ストレイフ移動)
					Vector3 forward = dir.Normalized();
					Vector3 right = Cross(forward, Vector3(0.0f, 1.0f, 0.0f));
					myTrans.position += right * sinf(moveTimer * 3.0f) * speed * 8.0f * dt;
				}

				// 攻撃タイマー更新
				attackTimer += dt;
				if (attackTimer >= attackCooldown)
				{
					targetPos.y += 1.0f; // 少し上を狙う
					Shoot(targetPos);
					attackTimer = 0.0f;
				}
			}
			break;
		}

		GetComponent<MeshRenderer>().pModel->Step(1.0f / fFPS);
	}

	// HPバー更新
	if (world->HasComponent<Transform>(hpBarId))
	{
		auto& myTrans = GetComponent<Transform>();
		Vector3 barPos = myTrans.position + Vector3(0.0f, HP_OFFSET, 0.0f);

		Quaternion q = Quaternion::Identity();
		ECS::EntityID camId = CameraSystem::GetCamera();
		if (world->HasComponent<Transform>(camId)) {
			q = world->GetComponent<Transform>(camId).rotation;
		}

		auto& fgT = world->GetComponent<Transform>(hpBarId);
		Vector3 right = q * Vector3(HP_LENGTH, 0.0f, 0.0f);
		fgT.position = barPos - (right * 0.5f);
		fgT.rotation = q;

		float ratio = (float)hp / (float)maxHp;
		if (ratio < 0.0f) ratio = 0.0f;
		fgT.scale.x = ratio * HP_LENGTH;
	}

	// ダメージ演出
	if (damageTimer > 0.0f)
	{
		damageTimer -= 1.0f / fFPS;
		if (world->HasComponent<MeshRenderer>(entityId))
		{
			auto& mr = world->GetComponent<MeshRenderer>(entityId);
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

void ShootingEnemy::Shoot(Vector3 targetPos)
{
	auto& myTrans = GetComponent<Transform>();
	Vector3 spawnPos = myTrans.position;
	Vector3 dir = (targetPos - spawnPos).Normalized();

	Push([spawnPos, dir](ECS::World& world) {
		ECS::EntityID id = world.CreateEntity();

		// Transform
		Transform t;
		t.position = spawnPos;
		t.scale = Vector3(0.2f, 0.2f, 0.2f);
		// 進行方向に向ける
		float radian = atan2(dir.x, dir.z);
		t.rotation = Quaternion::FromRotation(0.0f, DirectX::XMConvertToDegrees(radian), 0.0f);
		world.AddComponent<Transform>(id, t);

		// MeshRenderer (弾のモデル)
		MeshRenderer mr;
		mr.pModel = std::make_shared<Model>();
		mr.pModel->Load("Assets/Model/bullet.fbx"); // 既存の弾モデルを流用
		mr.pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		mr.pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		mr.color = Vector4(1.0f, 0.0f, 0.0f, 1.0f); // 赤色にする
		world.AddComponent<MeshRenderer>(id, mr);

		// Rigidbody
		Rigidbody rb;
		rb.useGravity = false;
		rb.velocity = dir * 0.2f; // 弾速
		world.AddComponent<Rigidbody>(id, rb);

		// Collider
		BoxCollider bc;
		bc.size = Vector3(0.2f, 0.2f, 0.2f);
		bc.isTrigger = true; // 物理衝突はさせない
		bc.tag = CollisionTag::ENEMY; // 敵の攻撃として扱う
		world.AddComponent<BoxCollider>(id, bc);

		// Script
		Script script;
		script.Bind<EnemyBullet>();
		world.AddComponent<Script>(id, script);
	});
}

void ShootingEnemy::OnCollisionEnter(ECS::EntityID other)
{
	auto& collider = world->GetComponent<BoxCollider>(other);

	// プレイヤーの弾と当たったらダメージ
	if (collider.tag == CollisionTag::BULLET)
	{
		hp -= PlayerManager::GetAttackPower();
		damageTimer = 0.5f;

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

ShootingEnemy::~ShootingEnemy()
{
	// シーン遷移時(World破棄時)にPushするとCommandBufferが既に破棄されている可能性があるため、
	// 撃破時(HP<=0)のみ削除コマンドを発行する
	if (hp <= 0) {
		if (hpBarBgId != -1) Push([id = hpBarBgId](ECS::World& w) { w.DeleteEntity(id); });
		if (hpBarId != -1) Push([id = hpBarId](ECS::World& w) { w.DeleteEntity(id); });
	}
}