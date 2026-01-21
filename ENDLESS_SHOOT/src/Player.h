// Player.h

#pragma once
#include "NativeScript.h"
#include "Vector.h"

class Player : public NativeScript
{
public:
	Player() = default;
	~Player() = default;

	void OnCreate() override;
	void Update() override;
	void OnDestroy() override;

	void Damage(float damage);
	
	static void SetSpawnPosition(const Vector3& pos);

private:

	ECS::EntityID weaponId = 0;
	ECS::EntityID hpBarId = 0;
	ECS::EntityID damageOverlayId = 0;
	float damageOverlayAlpha = 0.0f;
	float hp = 10;
	float maxHp = 10;
	bool isDead = false;
	static Vector3 spawnPosition;
};