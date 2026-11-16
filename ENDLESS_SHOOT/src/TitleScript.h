#pragma once
#include "NativeScript.h"

class TitleScript : public NativeScript
{
public:

	void OnCreate() override;
	void Update() override;

private:
	void UpdateCamera();

	float cameraYaw = 0.0f;
	float cameraPhase = 0.0f;
};