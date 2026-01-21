#include "SceneStage2.h"
#include "Geometory.h"
#include "Vector.h"
#include "EntityID.h"
#include "ShaderList.h"
#include "Component.h"
#include "Player.h"
#include "Enemy.h"
#include "EntityTag.h"
#include "ShootingEnemy.h"
#include "GameManager.h"
#include "Clear.h"
#include "Random.h"
#include "EnemyGenerator.h"
#include "BGMPlayer.h"

static constexpr float FLOOR_SCALE_X = 40.0f;
static constexpr float FLOOR_SCALE_Z = 40.0f;
static constexpr float STAGE_Y = 40.0f;

void SceneStage2::Init()
{
	world.RegisterStructural<Transform>();
	world.RegisterStructural<MeshRenderer>();
	world.RegisterStructural<BoxCollider>();
	world.RegisterStructural<Rigidbody>();
	world.RegisterStructural<SpriteRenderer>();

	GameManager::SetState(GameManager::PLAY);
	GameManager::AdvanceStage();

	{
		// プレイヤー
		ECS::EntityID entity = world.CreateEntity();
		Script script;
		script.Bind<Player>();
		world.AddComponent<Script>(entity, script);
		world.AddComponent<PlayerTag>(entity, PlayerTag());
	}

	{
		ECS::EntityID entity = world.CreateEntity();
		Script script;
		script.Bind<Clear>();
		world.AddComponent<Script>(entity, script);
	}

	// BGM再生用エンティティの作成
	{
		ECS::EntityID entity = world.CreateEntity();
		Script script;
		script.Bind<BGMPlayer>(0.3f);
		world.AddComponent<Script>(entity, script);
	}

	// --- 敵キャラクターの配置変更 ---
	// ステージ数に応じて敵を追加
	// 出現位置の候補リスト
	Vector3 spawnPositions[] = {
		Vector3(-12.0f, 1.0f, 5.0f),
		Vector3(12.0f, 1.0f, 5.0f),
		Vector3(-6.0f, 1.0f, 0.0f),
		Vector3(6.0f, 1.0f, 0.0f),
		Vector3(0.0f, 1.0f, -2.0f),
		Vector3(-12.0f, 1.0f, -5.0f),
		Vector3(12.0f, 1.0f, -5.0f),
		Vector3(-6.0f, 1.0f, -10.0f),
		Vector3(6.0f, 1.0f, -10.0f),
	};
	int maxEnemies = sizeof(spawnPositions) / sizeof(spawnPositions[0]);
	int spawnCount = GameManager::GetStageCount();
	if (spawnCount > maxEnemies) spawnCount = maxEnemies;

	for (int i = 0; i < spawnCount; ++i)
	{
		EnemyGenerator::Create(world, spawnPositions[i]);
	}

	{
		// 敵1 (左奥)
		ECS::EntityID entity = world.CreateEntity();
		world.AddComponent<Transform>(entity, Transform());
		world.GetComponent<Transform>(entity).position = Vector3(-8.0f, 1.0f, -8.0f);
		world.GetComponent<Transform>(entity).scale = Vector3(1.0f, 1.0f, 1.0f);

		Model* pModel = new Model();
		pModel->Load("Assets/Model/character_dog.fbx");
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		world.AddComponent<MeshRenderer>(entity, MeshRenderer(pModel));

		BoxCollider enemyCol(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.5f, 1.0f), Quaternion::Identity());
		enemyCol.tag = CollisionTag::ENEMY;
		enemyCol.isStatic = false;
		world.AddComponent<BoxCollider>(entity, enemyCol);

		Script enemyScript;
		enemyScript.Bind<Enemy>();
		world.AddComponent<Script>(entity, enemyScript);

		world.AddComponent<EnemyTag>(entity, EnemyTag());

		Rigidbody rb;
		world.AddComponent<Rigidbody>(entity, rb);
	}

	{
		// 敵2 (右奥)
		ECS::EntityID entity = world.CreateEntity();
		world.AddComponent<Transform>(entity, Transform());
		world.GetComponent<Transform>(entity).position = Vector3(8.0f, 1.0f, -8.0f);
		world.GetComponent<Transform>(entity).scale = Vector3(1.0f, 1.0f, 1.0f);

		Model* pModel = new Model();
		pModel->Load("Assets/Model/character_dog.fbx");
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		world.AddComponent<MeshRenderer>(entity, MeshRenderer(pModel));

		BoxCollider enemyCol(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.5f, 1.0f), Quaternion::Identity());
		enemyCol.tag = CollisionTag::ENEMY;
		enemyCol.isStatic = false;
		world.AddComponent<BoxCollider>(entity, enemyCol);

		Script enemyScript;
		enemyScript.Bind<Enemy>();
		world.AddComponent<Script>(entity, enemyScript);

		world.AddComponent<EnemyTag>(entity, EnemyTag());

		Rigidbody rb;
		world.AddComponent<Rigidbody>(entity, rb);
	}

	// 射撃する敵 (中央奥、障害物の裏)
	{
		ECS::EntityID entity = world.CreateEntity();
		Transform t;
		t.position = Vector3(0.0f, 1.0f, -15.0f); // 奥に配置
		t.scale = Vector3(1.0f, 1.0f, 1.0f);
		world.AddComponent<Transform>(entity, t);

		Model* pModel = new Model();
		pModel->Load("Assets/Model/Voletir.fbx");
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_ANIME));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		pModel->AddAnimation("Assets/Model/Anim_Voletir_06_OpenVault_Idle.fbx");
		MeshRenderer mr(pModel);
		world.AddComponent<MeshRenderer>(entity, mr);

		BoxCollider enemyCol(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.7f, 1.0f), Quaternion::Identity());
		enemyCol.tag = CollisionTag::ENEMY;
		enemyCol.isStatic = false;
		world.AddComponent<BoxCollider>(entity, enemyCol);

		Script enemyScript;
		enemyScript.Bind<ShootingEnemy>();
		world.AddComponent<Script>(entity, enemyScript);

		Rigidbody rb;
		rb.useGravity = false;
		world.AddComponent<Rigidbody>(entity, rb);

		world.AddComponent<EnemyTag>(entity, EnemyTag());
	}

	// --- 障害物の追加 ---
	// 中央の柱
	{
		ECS::EntityID entity = world.CreateEntity();
		Transform transform;
		transform.position = Vector3(0.0f, 1.0f, -5.0f);
		transform.scale = Vector3(2.0f, 3.0f, 2.0f);
		world.AddComponent<Transform>(entity, transform);

		BoxCollider boxCollider(Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 3.0f, 2.0f), Quaternion::Identity());
		boxCollider.isStatic = true;
		world.AddComponent<BoxCollider>(entity, boxCollider);

		Model* pModel = new Model();
		pModel->Load("Assets/Model/box.fbx");
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		MeshRenderer mr(pModel);
		mr.color = Vector4(0.6f, 0.6f, 0.7f, 1.0f);
		world.AddComponent<MeshRenderer>(entity, mr);
	}

	// 左側の壁
	{
		ECS::EntityID entity = world.CreateEntity();
		Transform transform;
		transform.position = Vector3(-10.0f, 0.5f, -2.0f);
		transform.scale = Vector3(1.0f, 2.0f, 6.0f);
		world.AddComponent<Transform>(entity, transform);

		BoxCollider boxCollider(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 2.0f, 6.0f), Quaternion::Identity());
		boxCollider.isStatic = true;
		world.AddComponent<BoxCollider>(entity, boxCollider);

		Model* pModel = new Model();
		pModel->Load("Assets/Model/box.fbx");
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		MeshRenderer mr(pModel);
		mr.color = Vector4(0.6f, 0.6f, 0.7f, 1.0f);
		world.AddComponent<MeshRenderer>(entity, mr);
	}

	// 右側の壁
	{
		ECS::EntityID entity = world.CreateEntity();
		Transform transform;
		transform.position = Vector3(10.0f, 0.5f, -2.0f);
		transform.scale = Vector3(1.0f, 2.0f, 6.0f);
		world.AddComponent<Transform>(entity, transform);

		BoxCollider boxCollider(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 2.0f, 6.0f), Quaternion::Identity());
		boxCollider.isStatic = true;
		world.AddComponent<BoxCollider>(entity, boxCollider);

		Model* pModel = new Model();
		pModel->Load("Assets/Model/box.fbx");
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		MeshRenderer mr(pModel);
		mr.color = Vector4(0.6f, 0.6f, 0.7f, 1.0f);
		world.AddComponent<MeshRenderer>(entity, mr);
	}

	// 追加の障害物 (3つ)
	Vector3 extraObstacles[] = {
		Vector3(-4.0f, 0.75f, 3.0f),
		Vector3(4.0f, 0.75f, 3.0f),
		Vector3(0.0f, 0.75f, 0.0f)
	};

	for (const auto& pos : extraObstacles)
	{
		ECS::EntityID entity = world.CreateEntity();
		Transform transform;
		transform.position = pos;
		transform.scale = Vector3(1.5f, 1.5f, 1.5f);
		world.AddComponent<Transform>(entity, transform);

		BoxCollider boxCollider(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.5f, 1.5f, 1.5f), Quaternion::Identity());
		boxCollider.isStatic = true;
		world.AddComponent<BoxCollider>(entity, boxCollider);

		Model* pModel = new Model();
		pModel->Load("Assets/Model/box.fbx");
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		world.AddComponent<MeshRenderer>(entity, MeshRenderer(pModel));
	}

	// --- 床と壁（SceneGameと同様） ---
	{
		// ゆか
		ECS::EntityID entity = world.CreateEntity();
		Transform transform;
		transform.position = Vector3(0.0f, -0.5f, 0.0f);
		transform.scale = Vector3(FLOOR_SCALE_X, 1.0f, FLOOR_SCALE_Z);
		world.AddComponent<Transform>(entity, transform);

		BoxCollider boxCollider(Vector3(0.0f, -0.5f, 0.0f), Vector3(FLOOR_SCALE_X, 1.0f, FLOOR_SCALE_Z), Quaternion::Identity());
		boxCollider.isStatic = true;
		world.AddComponent<BoxCollider>(entity, boxCollider);

		Model* pModel = new Model();
		pModel->Load("Assets/Model/box.fbx");
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		world.AddComponent<MeshRenderer>(entity, MeshRenderer(pModel));
	}

	// 落ちないようにするための壁
	{
		// +Z
		ECS::EntityID entity = world.CreateEntity();
		Transform transform;
		transform.position = Vector3(0.0f, -0.5f + STAGE_Y * 0.5f, FLOOR_SCALE_Z * 0.5f + 0.5f);
		transform.scale = Vector3(FLOOR_SCALE_X, STAGE_Y, 1.0f);
		world.AddComponent<Transform>(entity, transform);

		BoxCollider boxCollider(Vector3(0.0f, -0.5f, 0.0f), Vector3(FLOOR_SCALE_X, STAGE_Y, 1.0f), Quaternion::Identity());
		boxCollider.isStatic = true;
		world.AddComponent<BoxCollider>(entity, boxCollider);
	}
	{
		// -Z
		ECS::EntityID entity = world.CreateEntity();
		Transform transform;
		transform.position = Vector3(0.0f, -0.5f + STAGE_Y * 0.5f, -(FLOOR_SCALE_Z * 0.5f + 0.5f));
		transform.scale = Vector3(FLOOR_SCALE_X, STAGE_Y, 1.0f);
		world.AddComponent<Transform>(entity, transform);

		BoxCollider boxCollider(Vector3(0.0f, -0.5f, 0.0f), Vector3(FLOOR_SCALE_X, STAGE_Y, 1.0f), Quaternion::Identity());
		boxCollider.isStatic = true;
		world.AddComponent<BoxCollider>(entity, boxCollider);
	}
	{
		// +X
		ECS::EntityID entity = world.CreateEntity();
		Transform transform;
		transform.position = Vector3(FLOOR_SCALE_X * 0.5f + 0.5f, -0.5f + STAGE_Y * 0.5f, 0.0f);
		transform.scale = Vector3(1.0f, STAGE_Y, FLOOR_SCALE_X);
		world.AddComponent<Transform>(entity, transform);

		BoxCollider boxCollider(Vector3(0.0f, -0.5f, 0.0f), Vector3(1.0f, STAGE_Y, FLOOR_SCALE_X), Quaternion::Identity());
		boxCollider.isStatic = true;
		world.AddComponent<BoxCollider>(entity, boxCollider);
	}
	{
		// -X
		ECS::EntityID entity = world.CreateEntity();
		Transform transform;
		transform.position = Vector3(-(FLOOR_SCALE_X * 0.5f + 0.5f), -0.5f + STAGE_Y * 0.5f, 0.0f);
		transform.scale = Vector3(1.0f, STAGE_Y, FLOOR_SCALE_X);
		world.AddComponent<Transform>(entity, transform);

		BoxCollider boxCollider(Vector3(0.0f, -0.5f, 0.0f), Vector3(1.0f, STAGE_Y, FLOOR_SCALE_X), Quaternion::Identity());
		boxCollider.isStatic = true;
		world.AddComponent<BoxCollider>(entity, boxCollider);
	}

	{
		// 天井
		ECS::EntityID entity = world.CreateEntity();
		Transform transform;
		transform.position = Vector3(0.0f, -0.5f + STAGE_Y, 0.0f);
		transform.scale = Vector3(FLOOR_SCALE_X, 1.0f, FLOOR_SCALE_Z);
		world.AddComponent<Transform>(entity, transform);

		BoxCollider boxCollider(Vector3(0.0f, -0.5f, 0.0f), Vector3(FLOOR_SCALE_X, 1.0f, FLOOR_SCALE_Z), Quaternion::Identity());
		boxCollider.isStatic = true;
		world.AddComponent<BoxCollider>(entity, boxCollider);
	}
}
