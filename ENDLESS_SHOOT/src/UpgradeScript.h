#pragma once
#include "NativeScript.h"
#include "Audio.h"

class UpgradeScript : public NativeScript
{
public:
	void OnCreate() override;
	void Update() override;
	void OnDestroy() override;

private:

	int cursor = 0;
	int upgrades[3];
	ECS::EntityID cursorId = 0;
	soundHandle cursorSE = INVALID_SOUND_HANDLE;
	soundHandle decideSE = INVALID_SOUND_HANDLE;
};
