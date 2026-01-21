#pragma once
#include "NativeScript.h"
#include "Audio.h"
#include "Vector.h"

class FlyingEnemy : public NativeScript
{
public:
	FlyingEnemy();
	~FlyingEnemy();

	void OnCreate() override;
	void Update() override;
	void OnCollisionEnter(ECS::EntityID other) override;

private:
	float hp = 2;
	float maxHp = 2;
	float speed = 1.5f;
	float damageTimer = 0.0f;
	float hoverTimer = 0.0f;

	ECS::EntityID hpBarBgId = -1;

	ECS::EntityID hpBarId = -1;
	soundHandle deathSE;

	enum class State {
		WAIT,
		MOVE
	};
	State currentState = State::WAIT;
	float stateTimer = 0.0f;
	Vector3 moveDir;
};