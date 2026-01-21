#pragma once
#include "NativeScript.h"

class EnemyBullet : public NativeScript
{
public:
	void Update() override;
	void OnCollisionEnter(ECS::EntityID other) override;

private:
	float lifeTime = 5.0f; // õ–½
	float damage = 1.0f;   // ƒ_ƒ[ƒW—Ê
};
