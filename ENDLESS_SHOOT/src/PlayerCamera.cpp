#include "PlayerCamera.h"
#include "Input.h"
#include "Component.h"
#include "CameraSystem.h"
#include "Defines.h"

static constexpr float MAX_PITCH = 89.0f;
static constexpr float MIN_PITCH = -50.0f;

static constexpr float SENSITIVITY = 0.05;

PlayerCamera::PlayerCamera(ECS::EntityID _playerId):
	playerId(_playerId),
	pitch(0.0f),
	yaw(180.0f),
	sensitivity(SENSITIVITY)
{
}

void PlayerCamera::OnCreate()
{
	// マウスをロックして非表示にする
	SetMouseLock(true);

	ECS::EntityID ID = entityId;
	Push([ID](ECS::World& world) {
		world.AddComponent<Transform>(ID, Transform());
		world.AddComponent<Camera>(ID, Camera());
	});

	// メインカメラに設定
	CameraSystem::SetCamera(entityId);
}

void PlayerCamera::Update()
{
	// マウスの移動量を取得
	float dx = (float)GetMouseDeltaX();
	float dy = (float)GetMouseDeltaY();

	// 回転角度の更新
	yaw += dx * sensitivity;
	pitch += dy * sensitivity;

	// 上下の回転制限 (真上・真下まで行かないように)
	if (pitch > MAX_PITCH) pitch = MAX_PITCH;
	if (pitch < MIN_PITCH) pitch = MIN_PITCH;

	// カメラの回転を適用 (Pitch, Yaw)
	auto& transform = GetComponent<Transform>();
	transform.rotation = Quaternion::FromRotation(pitch, yaw, 0.0f);
}

void PlayerCamera::LateUpdate()
{
	auto& transform = GetComponent<Transform>();

	// プレイヤーの位置に追従
	if (world->HasComponent<Transform>(playerId))
	{
		auto& playerTrans = world->GetComponent<Transform>(playerId);

		// 目の高さオフセット (Y+1.5f)
		transform.position = playerTrans.position + Vector3(0.0f, 1.7f, 0.0f);

		Vector3 targetPos = playerTrans.position + Vector3(0.0f, 1.5f, 0.0f);

		// 移動中のカメラ揺れ (Head Bobbing)
		if (world->HasComponent<Rigidbody>(playerId))
		{
			auto& rb = world->GetComponent<Rigidbody>(playerId);
			// 水平方向の速度
			float speed = Vector3(rb.velocity.x, 0.0f, rb.velocity.z).Magnitude();

			if (speed > 0.01f)
			{
				bool isRunning = IsKeyPress(VK_SHIFT);
				float bobFreq = isRunning ? 15.0f : 11.0f; // 揺れの速さ
				float bobAmp  = isRunning ? 0.05f : 0.02f;  // 揺れの大きさ

				bobTimer += bobFreq * (1.0f / fFPS);
				targetPos.y += sinf(bobTimer) * bobAmp;
			}
			else
			{
				bobTimer = 0.0f;
			}
		}

		transform.position = targetPos;

		// プレイヤーの体もY軸回転 (Yaw) させる
		playerTrans.rotation = Quaternion::FromRotation(0.0f, yaw, 0.0f);
	}
}

void PlayerCamera::OnDestroy()
{
	// マウスロック解除
	SetMouseLock(false);
}
