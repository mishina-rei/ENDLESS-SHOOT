#pragma once
#include "NativeScript.h"
#include "Audio.h"

class Enemy : public NativeScript
{
public:

	Enemy();
	~Enemy();

	void OnCreate() override;
	void Update() override;
	void OnCollisionEnter(ECS::EntityID other) override;

private:
	float hp = 3;
	float maxHp = 3;
	float speed; // ÅgG????Åg?Åe?Ågx
	float damageTimer = 0.0f;

	enum class State {
		CHASE,
		ATTACK,
		WAIT
	};

	State currentState = State::CHASE;
	float waitTimer = 0.0f;

	ECS::EntityID hpBarBgId = -1;
	ECS::EntityID hpBarId = -1;
	soundHandle deathSE;
};