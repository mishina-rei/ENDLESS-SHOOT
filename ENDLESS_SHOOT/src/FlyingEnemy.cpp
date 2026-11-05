#include "FlyingEnemy.h"
#include "Component.h"
#include "Defines.h"
#include "ColliderSystem.h"
#include "GameManager.h"
#include "PlayerManager.h"
#include "EntityTag.h"
#include "ShaderList.h"
#include "CameraSystem.h"
#include "Player.h"
#include "Random.h"

static constexpr float HP_OFFSET = 1.8f;
static constexpr float HP_LENGTH = 0.8f;

static constexpr float SPEED_MAX = 11.0f;
static constexpr float SPEED_DEFAULT = 2.4f;

FlyingEnemy::FlyingEnemy():
	deathSE(INVALID_SOUND_HANDLE)
{
}

void FlyingEnemy::OnCreate()
{
	int stage = GameManager::GetStageCount();
	hp = 2 + (stage - 1) * 1.0f;
	maxHp = hp;

	// ステージ数に応じてスピードアップ
	speed = SPEED_DEFAULT + (stage - 1) * 0.25f;
	if (speed > SPEED_MAX) speed = SPEED_MAX;

	// HP Bar
	Push([this](ECS::World& world) {
		hpBarId = world.CreateEntity();
		Transform t;
		t.scale = Vector3(1.0f, 0.1f, 1.0f);
		world.AddComponent<Transform>(hpBarId, t);

		SpriteRenderer sr;
		sr.SetTexture("Assets/Texture/white.png");
		sr.size = Vector2{ 1.0f, 1.0f };
		sr.color = Vector4(0.0f, 0.5f, 1.0f, 1.0f); // 青色
		sr.pivot = Vector2{ 0.0f, 0.5f };
		sr.isUI = false;
		world.AddComponent<SpriteRenderer>(hpBarId, sr);
	});

	// アニメーション再生
	GetComponent<MeshRenderer>().pModel->Play(0, true);

	// 死亡音ロード
	deathSE = LoadSound("Assets/Sound/enemyDeath/DSGNImpt_EXPLOSION-Forced Shutdown_HY_PC-001.wav");
	SetVolume(deathSE, 0.8f);
}

void FlyingEnemy::Update()
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
		float dt = 1.0f / fFPS;

		// 状態遷移 (停止と移動を繰り返す)
		stateTimer -= dt;
		if (stateTimer <= 0.0f)
		{
			if (currentState == State::WAIT)
			{
				currentState = State::MOVE;
				stateTimer = 2.0f; // 移動時間

				// プレイヤーの少し上を目指す
				Vector3 destPos = targetPos;
				destPos.y += 1.5f;
				Vector3 dir = destPos - myTrans.position;
				float distance = dir.Magnitude();

				// 距離が近ければ直接向かう、遠ければ少しずらす
				if (distance < 6.0f)
				{
					moveDir = dir.Normalized();
				}
				else
				{
					float angleY = Random::Range(-40.0f, 40.0f);
					float angleX = Random::Range(-45.0f, 45.0f);
					moveDir = Quaternion::FromRotation(angleX, angleY, 0.0f) * dir.Normalized();
				}
			}
			else
			{
				currentState = State::WAIT;
				stateTimer = 1.0f; // 停止時間
			}
		}

		// 移動処理
		if (currentState == State::MOVE)
		{
			myTrans.position += moveDir * speed * dt;

			// 向き変更
			float radian = atan2(moveDir.x, moveDir.z);
			float degree = DirectX::XMConvertToDegrees(radian);
			myTrans.rotation = Quaternion::FromRotation(0.0f, degree, 0.0f);
		}

		// 上下にふわふわさせる
		hoverTimer += dt * 2.0f;
		myTrans.position.y += sinf(hoverTimer) * 0.005f;
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
			else mr.color = Vector4(0.5f, 0.5f, 1.0f, 1.0f); // 通常時は青みがかっている
		}
	}
	else
	{
		if (world->HasComponent<MeshRenderer>(entityId))
		{
			world->GetComponent<MeshRenderer>(entityId).color = Vector4(0.5f, 0.5f, 1.0f, 1.0f);
		}
	}

	// アニメーション更新
	GetComponent<MeshRenderer>().pModel->Step(1.0f / fFPS);
}

void FlyingEnemy::OnCollisionEnter(ECS::EntityID other)
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
	// プレイヤーに当たったらダメージを与える
	else if (collider.tag == CollisionTag::PLAYER)
	{
		// プレイヤーへのダメージ処理
		if (world->HasComponent<Script>(other)) {
			auto& scriptComp = world->GetComponent<Script>(other);
			if (scriptComp.script) {
				Player* player = dynamic_cast<Player*>(scriptComp.script.get());
				if (player) {
					player->Damage(1.0f); // 接触ダメージ
				}
			}
		}
	}
}

FlyingEnemy::~FlyingEnemy()
{
	if (hp <= 0) {
		if (hpBarId != -1) Push([id = hpBarId](ECS::World& w) { w.DeleteEntity(id); });
	}
}