#include "TitleScript.h"
#include "Input.h"
#include "SceneManager.h"
#include "SceneGame.h"
#include "GameManager.h"
#include "PlayerManager.h"
#include "Audio.h"

static soundHandle titleDecideSE = INVALID_SOUND_HANDLE;

void TitleScript::OnCreate()
{
	titleDecideSE = LoadSound("Assets/Sound/UI/SFX_FastUiClickImpact08.wav");
}

void TitleScript::Update()
{
	// ?G???^?[?L?[??Q?[???V?[????J??
	if (IsKeyTrigger(VK_RETURN))
	{
		Play(titleDecideSE);
		GameManager::ResetStage();
		PlayerManager::Init();
		SceneManager::GetInstance().SetScene<SceneGame>();
	}
}