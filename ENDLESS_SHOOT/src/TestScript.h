#pragma once

#include "NativeScript.h"

class TestScript : public NativeScript
{
public:
	TestScript() = default;
	~TestScript() = default;

	void OnCreate() override;
	void Update() override;

	void OnCollisionEnter(ECS::EntityID other) override;
};