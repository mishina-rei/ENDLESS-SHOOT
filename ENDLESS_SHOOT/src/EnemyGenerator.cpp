#include "EnemyGenerator.h"
#include "Random.h"
#include "Enemy.h"
#include "ShootingEnemy.h"
#include "FlyingEnemy.h"
#include "ShaderList.h"
#include "Component.h"
#include "EntityTag.h"

void EnemyGenerator::Create(ECS::World& world, const Vector3& position)
{
	int type = Random::Range(0, 2);

	ECS::EntityID entity = world.CreateEntity();
	Transform t;
	t.position = position;
	t.scale = Vector3(1.0f, 1.0f, 1.0f);
	world.AddComponent<Transform>(entity, t);

	if (type == 0)
	{
		// 通常の敵 (Dog)
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

		Rigidbody rb;
		world.AddComponent<Rigidbody>(entity, rb);
	}
	else if (type == 1)
	{
		// 射撃する敵 (Voletir)
		Model* pModel = new Model();
		pModel->Load("Assets/Model/Voletir.fbx");
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_ANIME));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		pModel->AddAnimation("Assets/Model/Anim_Voletir_06_OpenVault_Idle.fbx");
		world.AddComponent<MeshRenderer>(entity, MeshRenderer(pModel));

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
	}
	else
	{
		// 飛行する敵 (FlyingEnemy)
		Model* pModel = new Model();
		pModel->Load("Assets/Model/Voletir.fbx"); // モデルは流用
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_ANIME));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		pModel->AddAnimation("Assets/Model/Anim_Voletir_06_OpenVault_Idle.fbx");
		MeshRenderer mr(pModel);
		mr.color = Vector4(0.5f, 0.5f, 1.0f, 1.0f); // 青っぽくする
		world.AddComponent<MeshRenderer>(entity, mr);

		BoxCollider enemyCol(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f), Quaternion::Identity());
		enemyCol.tag = CollisionTag::ENEMY;
		enemyCol.isStatic = false;
		world.AddComponent<BoxCollider>(entity, enemyCol);

		Script enemyScript;
		enemyScript.Bind<FlyingEnemy>();
		world.AddComponent<Script>(entity, enemyScript);

		Rigidbody rb;
		rb.useGravity = false;
		world.AddComponent<Rigidbody>(entity, rb);
	}

	world.AddComponent<EnemyTag>(entity, EnemyTag());
}
