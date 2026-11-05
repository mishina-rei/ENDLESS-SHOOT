#include "SceneStage4.h"
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
#include "EnemyGenerator.h"
#include "BGMPlayer.h"

static constexpr float FLOOR_SCALE_X = 40.0f;
static constexpr float FLOOR_SCALE_Z = 40.0f;
static constexpr float STAGE_Y = 40.0f;

void SceneStage4::Init()
{
	world.RegisterStructural<Transform>();
	world.RegisterStructural<MeshRenderer>();
	world.RegisterStructural<BoxCollider>();
	world.RegisterStructural<Rigidbody>();
	world.RegisterStructural<SpriteRenderer>();

	GameManager::SetState(GameManager::PLAY);
	GameManager::AdvanceStage();

	// プレイヤー位置設定 (少し左寄り)
	Player::SetSpawnPosition(Vector3(-5.0f, 1.0f, 18.0f));

	{
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
	// --- 敵の配置 ---
	// 中央の壁の裏に射撃敵
	Vector3 enemyPositions[] = {
		Vector3(0.0f, 1.0f, -10.0f),
		Vector3(-10.0f, 1.0f, 0.0f),
		Vector3(10.0f, 1.0f, 0.0f),
		Vector3(-15.0f, 1.0f, -5.0f),
		Vector3(15.0f, 1.0f, -5.0f),
		Vector3(-5.0f, 1.0f, -15.0f),
		Vector3(5.0f, 1.0f, -15.0f),
	};
	int spawnCount = 3 + GameManager::GetStageCount();
	int maxEnemies = sizeof(enemyPositions) / sizeof(enemyPositions[0]);
	if (spawnCount > maxEnemies) spawnCount = maxEnemies;
	for (int i = 0; i < spawnCount; ++i) EnemyGenerator::Create(world, enemyPositions[i]);

	// --- 障害物 (中央の巨大な壁) ---
	{
		ECS::EntityID entity = world.CreateEntity();
		Transform transform;
		transform.position = Vector3(0.0f, 2.0f, 0.0f);
		transform.scale = Vector3(10.0f, 4.0f, 2.0f);
		world.AddComponent<Transform>(entity, transform);

		BoxCollider boxCollider(Vector3(0.0f, 0.0f, 0.0f), Vector3(10.0f, 4.0f, 2.0f), Quaternion::Identity());
		boxCollider.isStatic = true;
		world.AddComponent<BoxCollider>(entity, boxCollider);

		Model* pModel = new Model();
		pModel->Load("Assets/Model/box.fbx");
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		MeshRenderer mr(pModel);
		mr.color = Vector4(0.7f, 0.4f, 0.4f, 1.0f);
		world.AddComponent<MeshRenderer>(entity, mr);
	}

	// 追加の障害物 (3つ)
	Vector3 extraObstacles[] = {
		Vector3(-8.0f, 1.0f, 10.0f),
		Vector3(8.0f, 1.0f, 10.0f),
		Vector3(0.0f, 1.0f, 12.0f)
	};

	for (const auto& pos : extraObstacles)
	{
		ECS::EntityID entity = world.CreateEntity();
		Transform transform;
		transform.position = pos;
		transform.scale = Vector3(2.0f, 2.0f, 2.0f);
		world.AddComponent<Transform>(entity, transform);

		BoxCollider boxCollider(Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f), Quaternion::Identity());
		boxCollider.isStatic = true;
		world.AddComponent<BoxCollider>(entity, boxCollider);

		Model* pModel = new Model();
		pModel->Load("Assets/Model/box.fbx");
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		world.AddComponent<MeshRenderer>(entity, MeshRenderer(pModel));
	}

	// --- 床と壁 ---
	{
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

	auto CreateWall = [&](Vector3 pos, Vector3 scale) {
		ECS::EntityID entity = world.CreateEntity();
		Transform transform;
		transform.position = pos;
		transform.scale = scale;
		world.AddComponent<Transform>(entity, transform);
		BoxCollider boxCollider(Vector3(0.0f, -0.5f, 0.0f), scale, Quaternion::Identity());
		boxCollider.isStatic = true;
		world.AddComponent<BoxCollider>(entity, boxCollider);
	};

	CreateWall(Vector3(0.0f, -0.5f + STAGE_Y * 0.5f, FLOOR_SCALE_Z * 0.5f + 0.5f), Vector3(FLOOR_SCALE_X, STAGE_Y, 1.0f));
	CreateWall(Vector3(0.0f, -0.5f + STAGE_Y * 0.5f, -(FLOOR_SCALE_Z * 0.5f + 0.5f)), Vector3(FLOOR_SCALE_X, STAGE_Y, 1.0f));
	CreateWall(Vector3(FLOOR_SCALE_X * 0.5f + 0.5f, -0.5f + STAGE_Y * 0.5f, 0.0f), Vector3(1.0f, STAGE_Y, FLOOR_SCALE_X));
	CreateWall(Vector3(-(FLOOR_SCALE_X * 0.5f + 0.5f), -0.5f + STAGE_Y * 0.5f, 0.0f), Vector3(1.0f, STAGE_Y, FLOOR_SCALE_X));
	CreateWall(Vector3(0.0f, -0.5f + STAGE_Y, 0.0f), Vector3(FLOOR_SCALE_X, 1.0f, FLOOR_SCALE_Z));
}