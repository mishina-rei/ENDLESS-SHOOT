#include "TitleScript.h"
#include "Input.h"
#include "SceneManager.h"
#include "SceneGame.h"
#include "GameManager.h"
#include "PlayerManager.h"
#include "Audio.h"
#include "Component.h"
#include "Time.h"
#include <cmath>

namespace
{
	constexpr float CAMERA_YAW_SPEED = 6.0f;
	constexpr float CAMERA_BASE_HEIGHT = 4.0f;
	constexpr float CAMERA_HEIGHT_AMPLITUDE = 0.3f;
	constexpr float CAMERA_MOVE_RADIUS = 1.5f;
	constexpr float CAMERA_SWAY_PERIOD = 20.0f;
}

static soundHandle titleDecideSE = INVALID_SOUND_HANDLE;

void TitleScript::OnCreate()
{
	titleDecideSE = LoadSound("Assets/Sound/UI/SFX_FastUiClickImpact08.wav");
}

void TitleScript::Update()
{
	UpdateCamera();

	// エンターキーでゲームシーンへ遷移
	if (IsKeyTrigger(VK_RETURN))
	{
		Play(titleDecideSE);
		GameManager::ResetStage();
		PlayerManager::Init();
		SceneManager::GetInstance().SetScene<SceneGame>();
	}
}

void TitleScript::UpdateCamera()
{
	const float deltaTime = Time::GetUnscaledDeltaTime();

	cameraYaw = std::fmod(
		cameraYaw + CAMERA_YAW_SPEED * deltaTime,
		360.0f
	);

	cameraPhase = std::fmod(
		cameraPhase + DirectX::XM_2PI / CAMERA_SWAY_PERIOD * deltaTime,
		DirectX::XM_2PI
	);

	auto& transform = GetComponent<Transform>();
	transform.position = Vector3(
		std::sin(cameraPhase) * CAMERA_MOVE_RADIUS,
		CAMERA_BASE_HEIGHT + std::sin(cameraPhase) * CAMERA_HEIGHT_AMPLITUDE,
		std::cos(cameraPhase) * CAMERA_MOVE_RADIUS
	);

	const float pitch = -5.0f + std::sin(cameraPhase) * 2.0f;
	transform.rotation = Quaternion::FromRotation(pitch, cameraYaw, 0.0f);
}
