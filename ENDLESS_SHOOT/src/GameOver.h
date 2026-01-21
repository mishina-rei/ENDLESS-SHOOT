#pragma once

#include "NativeScript.h"
#include "Audio.h"

class GameOver :public NativeScript
{
public:

	void OnCreate()override;
	void Update()override;

private:
	soundHandle decideSE = INVALID_SOUND_HANDLE;
};