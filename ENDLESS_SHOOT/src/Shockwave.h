#pragma once
#include "NativeScript.h"

class Shockwave : public NativeScript
{
public:
	void Update() override;
	void OnCollisionEnter(ECS::EntityID other) override;

private:
	float lifeTime = 0.5f; // 0.5ïbÇ≈è¡ñ≈
};