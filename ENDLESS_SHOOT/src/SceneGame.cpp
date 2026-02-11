#include "SceneGame.h"
#include "Geometory.h"
#include "Vector.h"
#include "EntityID.h"
#include "ShaderList.h"
#include "TestScript.h"
#include "Component.h"
#include "Player.h"
#include "Enemy.h"
#include "EntityTag.h"
#include "ShootingEnemy.h"
#include "GameManager.h"
#include "Clear.h"
#include "BGMPlayer.h"

static constexpr float FLOOR_SCALE_X = 40.0f;
static constexpr float FLOOR_SCALE_Z = 40.0f;
static constexpr float STAGE_Y = 40.0f;

void SceneGame::Init()
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

	// ?G???e?B?e?B???
	{
		// 敵キャラクター
		ECS::EntityID entity = world.CreateEntity();
		Transform t;
		t.position = Vector3(0.0f, 1.0f, 5.0f); // 少し離れた位置に配置
		world.AddComponent<Transform>(entity, t);

		Model* pModel = new Model();
		pModel->Load("Assets/Model/character_dog.fbx");
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		world.AddComponent<MeshRenderer>(entity, MeshRenderer(pModel));

		// EnemyスクリプトとColliderを追加
		BoxCollider enemyCol(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 2.0f, 1.0f), Quaternion::Identity());
		enemyCol.tag = CollisionTag::ENEMY;
		enemyCol.isStatic = false; // 衝突イベントを受け取るため
		world.AddComponent<BoxCollider>(entity, enemyCol);

		Script enemyScript;
		enemyScript.Bind<Enemy>();
		world.AddComponent<Script>(entity, enemyScript);

		world.AddComponent<EnemyTag>(entity, EnemyTag());

		Rigidbody rb;
		world.AddComponent<Rigidbody>(entity, rb);
	}

	// 射撃する敵キャラクターの作成
	{
		ECS::EntityID entity = world.CreateEntity();
		Transform t;
		t.position = Vector3(5.0f, 1.0f, -5.0f); // 少し離れた位置に配置
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
		rb.useGravity = false; // 重力無効
		world.AddComponent<Rigidbody>(entity, rb);

		world.AddComponent<EnemyTag>(entity, EnemyTag());
	}

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
		transform.position = Vector3(FLOOR_SCALE_X * 0.5f + 0.5f, -0.5f + STAGE_Y * 0.5f,0.0f );
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
