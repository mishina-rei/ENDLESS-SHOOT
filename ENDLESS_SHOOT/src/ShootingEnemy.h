#pragma once
#include "NativeScript.h"
#include "Vector.h"
#include "Audio.h"

class ShootingEnemy : public NativeScript
{
public:
	ShootingEnemy();
	~ShootingEnemy();

	void OnCreate() override;
	void Update() override;
	void OnCollisionEnter(ECS::EntityID other) override;

private:
	void Shoot(Vector3 targetPos);

	float hp = 3;
	float maxHp = 3;
	float speed;
	float damageTimer = 0.0f;

	// ËŒ‚ŠÖ˜A
	float attackRange = 10.0f; // Ë’ö‹——£
	float attackCooldown = 2.0f; // UŒ‚ŠÔŠu
	float attackTimer = 0.0f;
	float moveTimer = 0.0f;

	enum class State {
		CHASE,  // ’Ç‚¢‚©‚¯‚é
		ATTACK, // UŒ‚iËŒ‚j
	};

	State currentState = State::CHASE;

	ECS::EntityID hpBarBgId = -1;
	ECS::EntityID hpBarId = -1;
	soundHandle deathSE;
};
