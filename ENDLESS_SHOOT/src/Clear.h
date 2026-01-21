/*****************************************************************//**
 * @file   Clear.h
 * @brief  クリア演出とシーン遷移
 *
 * @author
 * @date
 *********************************************************************/

#pragma once
#include "NativeScript.h"
#include "Audio.h"

class Clear : public NativeScript
{
public:

	Clear();
	void Update() override;

private:

	bool isClear = false;
	soundHandle clearDecideSE = INVALID_SOUND_HANDLE;
};