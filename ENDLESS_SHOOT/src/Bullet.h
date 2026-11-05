#pragma once
#include "NativeScript.h"

class Bullet : public NativeScript
{
public:
	Bullet(ECS::EntityID _gunId) : gunId(_gunId) {}
	void Update() override;
	void OnCollisionEnter(ECS::EntityID other) override;

private:
	float lifeTime = 3.0f; // 3ïbå„Ç…è¡ñ≈
	ECS::EntityID gunId;
};