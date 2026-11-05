#include "TestScript.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "ShaderList.h"
#include "Collider.h"
#include "Rigidbody.h"
#include "Input.h"

void TestScript::OnCreate()
{
	ECS::EntityID ID = entityId;
	Push([ID](ECS::World& w) {
		Transform transform;
		transform.position = Vector3(0.0f, 2.0f, 2.0f);
		w.AddComponent<Transform>(ID, transform);
		w.GetComponent<Transform>(ID).rotation = Quaternion(0.0f,90.0f,0.0f);
		MeshRenderer mr;
		mr.pModel = std::make_shared<Model>();
		mr.pModel->Load("Assets/Model/Char_Ronin_01.fbx");
		mr.pModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		mr.pModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		mr.pModel->AddAnimation("Assets/Model/Char_Ronin_01.fbx");

		w.AddComponent<MeshRenderer>(ID,mr);

		// Rigidbodyを追加
		Rigidbody rb;
		w.AddComponent<Rigidbody>(ID, rb);

		BoxCollider boxCollider(Vector3(0.0f, 0.5f, 0.0f), Vector3(1.0f, 1.0f, 1.0f), Quaternion::Identity(), false);
		boxCollider.isStatic = false;
		w.AddComponent<BoxCollider>(ID, boxCollider);
	});
}

void TestScript::Update()
{
	auto& transform = GetComponent<Transform>();
	transform.Rotate(Quaternion(0.0f, 0.1f, 0.0f));
	
	// Rigidbodyが追加されているか確認
	if (world->HasComponent<Rigidbody>(entityId))
	{
		if (IsKeyTrigger(VK_SPACE))
		{
			auto& rb = GetComponent<Rigidbody>();
			rb.velocity.y = 0.2f; // ジャンプ
		}
	}
}

void TestScript::OnCollisionEnter(ECS::EntityID other)
{
	auto& model = GetComponent<MeshRenderer>();


	if (world->HasComponent<MeshRenderer>(other))
	{
		world->RemoveComponent<MeshRenderer>(other);
	}
}
