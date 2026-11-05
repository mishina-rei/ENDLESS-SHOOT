#pragma once

#include "NativeScript.h"

class PlayerCamera : public NativeScript
{
public:

	PlayerCamera(ECS::EntityID _playerId);
	void OnCreate() override;
	void Update() override;
	void LateUpdate() override;
	void OnDestroy() override;

private:

	ECS::EntityID playerId;

	float pitch;
	float yaw;
	float sensitivity;
	float bobTimer = 0.0f;
};
