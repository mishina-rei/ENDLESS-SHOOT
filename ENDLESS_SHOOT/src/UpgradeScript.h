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

	int cursor = 0;									// アップグレードを選択するカーソルの位置
	int upgrades[3];								// 表示するアップグレードの番号
	ECS::EntityID cursorId = 0;						// カーソル用のエンティティID
	soundHandle cursorSE = INVALID_SOUND_HANDLE;	// カーソル移動効果音
	soundHandle decideSE = INVALID_SOUND_HANDLE;	// 決定音
};
