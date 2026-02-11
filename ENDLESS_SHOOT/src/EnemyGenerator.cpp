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
		// í èÌÇÃìG (Dog)
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
		// éÀåÇÇ∑ÇÈìG (Voletir)
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
		// îÚçsÇ∑ÇÈìG (FlyingEnemy)
		Model* pModel = new Model();
		pModel->Load("Assets/Model/Voletir.fbx"); // ÉÇÉfÉãÇÕó¨óp
		pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_ANIME));
		pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		pModel->AddAnimation("Assets/Model/Anim_Voletir_06_OpenVault_Idle.fbx");
		MeshRenderer mr(pModel);
		mr.color = Vector4(0.5f, 0.5f, 1.0f, 1.0f); // ê¬Ç¡Ç€Ç≠Ç∑ÇÈ
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
