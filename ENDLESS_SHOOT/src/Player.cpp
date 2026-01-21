#include "Player.h"
#include "Component.h"
#include "ShaderList.h"
#include "PlayerCamera.h"
#include "Input.h"
#include "Gun.h"
#include "ColliderSystem.h"
#include "GameOver.h"
#include "GameManager.h"
#include "PlayerManager.h"
#include "Defines.h"

static constexpr float HP_POS_X = 20.0f;
static constexpr float HP_POS_Y = 30.0f;

static constexpr float PLAYER_LENGTH = 1.7f;

static constexpr float PLAYER_SPEED = 0.05f;
static constexpr float PLAYER_RUN_SPEED = 0.09f;

static constexpr float DEFALUT_HP = 10.0f;

Vector3 Player::spawnPosition = Vector3(0.0f, 1.0f, 10.0f);

void Player::SetSpawnPosition(const Vector3& pos)
{
	spawnPosition = pos;
}

void Player::OnCreate()
{	
	ECS::EntityID ID = this->entityId;
	Push([this, ID](ECS::World& world)
		{
			// Transform?R???|?[?l???g?????
			Transform transform;
			transform.position = spawnPosition;
			world.AddComponent<Transform>(ID, transform);

			// Rigidbody?????
			Rigidbody rb;
			world.AddComponent<Rigidbody>(ID, rb);

			// BoxCollider?????
			BoxCollider boxCollider(Vector3(0.0f, PLAYER_LENGTH * 0.5f, 0.0f), Vector3(1.0f, PLAYER_LENGTH, 1.0f), Quaternion::Identity(), false);
			boxCollider.isStatic = false;
			boxCollider.tag = CollisionTag::PLAYER;
			world.AddComponent<BoxCollider>(ID, boxCollider);

			// カメラ関係
			ECS::EntityID cameraid = world.CreateEntity();

			Script cameraScript;
			cameraScript.Bind<PlayerCamera>(ID);
			world.AddComponent<Script>(cameraid, cameraScript);

			// 武器作る
			{
				ECS::EntityID id = world.CreateEntity();

				Script script;
				script.Bind<Gun>(cameraid);
				world.AddComponent<Script>(id, script);
			}

			// HPバーUIの作成
			// 背景
			{
				ECS::EntityID bgId = world.CreateEntity();
				Transform t;
				t.position = Vector3(HP_POS_X, HP_POS_Y, 0.0f);
				world.AddComponent<Transform>(bgId, t);

				SpriteRenderer sr;
				sr.SetTexture("Assets/Texture/white.png"); 
				sr.size = Vector2{200.0f, 20.0f};
				sr.color = Vector4(0.2f, 0.2f, 0.2f, 1.0f); // 背景は暗いグレー
				sr.pivot = Vector2{0.0f, 0.0f}; // 左上基準
				sr.isUI = true;
				world.AddComponent<SpriteRenderer>(bgId, sr);
			}
			// 前面（緑バー）
			{
				this->hpBarId = world.CreateEntity();
				Transform t;
				t.position = Vector3(HP_POS_X, HP_POS_Y, 0.0f);
				world.AddComponent<Transform>(this->hpBarId, t);

				SpriteRenderer sr;
				sr.SetTexture("Assets/Texture/white.png");
				sr.size = Vector2{200.0f, 20.0f};
				sr.color = Vector4(0.0f, 1.0f, 0.0f, 1.0f); // 緑
				sr.pivot = Vector2{0.0f, 0.0f}; // 左上基準
				sr.isUI = true;
				world.AddComponent<SpriteRenderer>(this->hpBarId, sr);
			}

			// ダメージ演出用オーバーレイ (画面全体を赤くする)
			{
				this->damageOverlayId = world.CreateEntity();
				Transform t;
				t.position = Vector3(SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0.0f);
				world.AddComponent<Transform>(this->damageOverlayId, t);

				SpriteRenderer sr;
				sr.SetTexture("Assets/Texture/damageUI.png");
				sr.color = Vector4(1.0f, 1.0f, 1.0f, 0.0f); // 赤色、最初は透明
				sr.isUI = true;
				sr.layer = SpriteLayer::UI_Front; // 前面に表示
				world.AddComponent<SpriteRenderer>(this->damageOverlayId, sr);
			}
		});
	
	maxHp = PlayerManager::GetMaxHp();
	hp = maxHp;
	isDead = false;
	damageOverlayAlpha = 0.0f;
}

void Player::Update()
{
	// HPバーの更新
	if (hpBarId != 0 && world->HasComponent<Transform>(hpBarId))
	{
		auto& t = world->GetComponent<Transform>(hpBarId);
		float ratio = hp / maxHp;
		if (ratio < 0.0f) ratio = 0.0f;
		t.scale.x = ratio;
	}

	// ダメージオーバーレイのフェードアウト
	if (damageOverlayAlpha > 0.0f)
	{
		damageOverlayAlpha -= 2.0f * (1.0f / fFPS); // フェードアウト速度
		if (damageOverlayAlpha < 0.0f) damageOverlayAlpha = 0.0f;

		if (damageOverlayId != 0 && world->HasComponent<SpriteRenderer>(damageOverlayId))
		{
			world->GetComponent<SpriteRenderer>(damageOverlayId).color.w = damageOverlayAlpha;
		}
	}

	// 死亡している場合は処理しない
	if (isDead) return;

	// Rigidbodyがない場合は処理しない
	if (!world->HasComponent<Rigidbody>(entityId)) return;

	auto& rb = GetComponent<Rigidbody>();
	auto& transform = GetComponent<Transform>();

	// 移動速度
	Vector3 moveDir = { 0.0f, 0.0f, 0.0f };

	// 入力
	if (IsKeyPress('W')) moveDir.z += 1.0f;
	if (IsKeyPress('S')) moveDir.z -= 1.0f;
	if (IsKeyPress('A')) moveDir.x -= 1.0f;
	if (IsKeyPress('D')) moveDir.x += 1.0f;

	// 移動入力がある場合
	if (moveDir.MagnitudeSq() > 0.0f)
	{
		float speed = IsKeyPress(VK_SHIFT) ? PLAYER_RUN_SPEED : PLAYER_SPEED;

		moveDir = moveDir.Normalized();

		// プレイヤーの向きに合わせて移動ベクトルを回転
		moveDir = transform.rotation * moveDir;

		rb.velocity.x = moveDir.x * speed;
		rb.velocity.z = moveDir.z * speed;
	}
	else
	{
		rb.velocity.x = 0.0f;
		rb.velocity.z = 0.0f;
	}

	// ジャンプ
	if (IsKeyTrigger(VK_SPACE)) {
		ColliderSystem::Ray ray;
		ray.origin = transform.position;
		ray.origin.y += 0.5f;
		ray.direction = Vector3(0.0f, -1.0f, 0.0f);
		ColliderSystem::RaycastHit hit;
		if (ColliderSystem::Instance().Raycast(world, ray, &hit, 0.6f, entityId)) {
			rb.velocity.y = 0.2f;
		}
	}
}

void Player::Damage(float damage)
{
	if (isDead) return;

	hp -= damage;

	// ダメージ演出 (赤くフラッシュさせる)
	damageOverlayAlpha = 0.5f; // 最大不透明度
	if (damageOverlayId != 0 && world->HasComponent<SpriteRenderer>(damageOverlayId))
	{
		world->GetComponent<SpriteRenderer>(damageOverlayId).color.w = damageOverlayAlpha;
	}

	if (hp <= 0)
	{
		hp = 0;
		isDead = true;

		// PLAY状態でなければゲームオーバーにしない
		if (GameManager::GetState() != GameManager::PLAY)
			return;

		GameManager::SetState(GameManager::GAMEOVER);

		// ゲームオーバー演出 (暗転 + テキスト)
		Push([](ECS::World& w) {

			ECS::EntityID id = w.CreateEntity();

			Script script;
			script.Bind<GameOver>();

			w.AddComponent<Script>(id,script);
		});
	}
}

void Player::OnDestroy()
{
	if (damageOverlayId != 0) {
		ECS::EntityID id = damageOverlayId;
		Push([id](ECS::World& w) { w.DeleteEntity(id); });
	}
	if (hpBarId != 0) {
		ECS::EntityID id = hpBarId;
		Push([id](ECS::World& w) { w.DeleteEntity(id); });
	}
}
