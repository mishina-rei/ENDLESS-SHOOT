#include "SceneStage12.h"
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
#include "Random.h"
#include "BGMPlayer.h"

static constexpr float FLOOR_SCALE_X = 40.0f;
static constexpr float FLOOR_SCALE_Z = 40.0f;
static constexpr float STAGE_Y = 40.0f;

void SceneStage12::Init()
{
	world.RegisterStructural<Transform>();
	world.RegisterStructural<MeshRenderer>();
	world.RegisterStructural<BoxCollider>();
	world.RegisterStructural<Rigidbody>();
	world.RegisterStructural<SpriteRenderer>();

	GameManager::SetState(GameManager::PLAY);
	GameManager::AdvanceStage();

	// プレイヤー位置設定
	Player::SetSpawnPosition(Vector3(15.0f, 1.0f, 15.0f));

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

	// --- 敵の配置 (大量) ---
	int spawnCount = 8 + GameManager::GetStageCount();
	for (int i = 0; i < spawnCount; ++i)
	{
		float x = Random::Range(-18.0f, 18.0f);
		float z = Random::Range(-18.0f, 18.0f);
		// プレイヤー付近は避ける
		if ((Vector3(x, 0.0f, z) - Vector3(15.0f, 0.0f, 15.0f)).Magnitude() < 10.0f) {
			x = -x; z = -z;
		}
		EnemyGenerator::Create(world, Vector3(x, 1.0f, z));
	}

	// --- 障害物 (ランダムブロック) ---
	auto CreateBlock = [&](Vector3 pos, Vector3 scale) {
		ECS::EntityID entity = world.CreateEntity();
		Transform transform;
		transform.position = pos;
		transform.scale = scale;
		world.AddComponent<Transform>(entity, transform);

		BoxCollider boxCollider(Vector3(0.0f, 0.0f, 0.0f), scale, Quaternion::Identity());
		boxCollider.isStatic = true;
		world.AddComponent<BoxCollider>(entity, boxCollider);

		Model* pModel = new Model();
		pModel->Load("Assets/Model/box.fbx");
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		MeshRenderer mr(pModel);
		mr.color = Vector4(Random::Range(0.2f, 0.8f), Random::Range(0.2f, 0.8f), Random::Range(0.2f, 0.8f), 1.0f);
		world.AddComponent<MeshRenderer>(entity, mr);
	};

	// ランダムに大小様々なブロックを配置
	for (int i = 0; i < 15; ++i) {
		float x = Random::Range(-15.0f, 15.0f);
		float z = Random::Range(-15.0f, 15.0f);
		float sx = Random::Range(1.0f, 5.0f);
		float sz = Random::Range(1.0f, 5.0f);
		float h = Random::Range(1.0f, 4.0f);
		
		// プレイヤー付近は避ける
		if ((Vector3(x, 0.0f, z) - Vector3(15.0f, 0.0f, 15.0f)).Magnitude() < 5.0f) continue;

		CreateBlock(Vector3(x, h * 0.5f, z), Vector3(sx, h, sz));
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