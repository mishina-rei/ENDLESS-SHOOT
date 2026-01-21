#include "UpgradeScript.h"
#include <vector>
#include <algorithm>
#include "Input.h"
#include "SceneManager.h"
#include "SceneGame.h"
#include "PlayerManager.h"
#include "Component.h"
#include "Defines.h"
#include "Random.h"
#include "StageGenerator.h"

static const float START_X = SCREEN_WIDTH * 0.2f;
static const float STEP_X = SCREEN_WIDTH * 0.3f;
static const float POS_Y = SCREEN_HEIGHT * 0.5f;

void UpgradeScript::OnCreate()
{
	// 背景 (半透明の黒)
	Push([](ECS::World& w) {
		ECS::EntityID bg = w.CreateEntity();
		Transform t;
		t.position = Vector3(SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0.0f);
		w.AddComponent<Transform>(bg, t);
		SpriteRenderer sr;
		sr.SetTexture("Assets/Texture/white.png");
		sr.size = Vector2({(float)SCREEN_WIDTH, (float)SCREEN_HEIGHT});
		sr.color = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
		sr.isUI = true;
		w.AddComponent<SpriteRenderer>(bg, sr);
	});

	// テキスト
		Push([](ECS::World& w) {
		ECS::EntityID bg = w.CreateEntity();
		Transform t;
		t.position = Vector3(SCREEN_WIDTH * 0.5f, 150.0f, 0.0f);
		w.AddComponent<Transform>(bg, t);
		SpriteRenderer sr;
		sr.SetTexture("Assets/Texture/selectUpgrade.png");
		sr.isUI = true;
		w.AddComponent<SpriteRenderer>(bg, sr);
			});

	// アップグレードをランダムで出す
	std::vector<int> candidates;
	for (int i = 0; i < PlayerManager::DATA_MAX; ++i)
	{
		candidates.push_back(i);
	}

	// ?????
	for (int i = 0; i < candidates.size(); ++i)
	{
		int j = Random::Range(i, (int)candidates.size() - 1);
		std::swap(candidates[i], candidates[j]);
	}

	// 3???
	for (int i = 0; i < 3; ++i)
	{
		upgrades[i] = (i < candidates.size()) ? candidates[i] : 0;
	}

	// ????3?????????
	float posY = SCREEN_HEIGHT * 0.5f;

	for (int i = 0; i < 3; ++i)
	{
		int type = upgrades[i];
        SpriteRenderer sr;
        sr.isUI = true;

		switch (type)
		{
		case PlayerManager::MAX_HP:
			sr.SetTexture("Assets/Texture/HP.png");
			break;
		case PlayerManager::ATTACK_POWER:
			sr.SetTexture("Assets/Texture/attack.png");
			break;
		case PlayerManager::FIRE_RATE:
			sr.SetTexture("Assets/Texture/fireRate.png");
			break;
		case PlayerManager::MAGAZINE_SIZE:
			sr.SetTexture("Assets/Texture/magazine.png");
			break;
		}

		Push([ i, sr](ECS::World& w) {
			ECS::EntityID id = w.CreateEntity();
			Transform t;
			t.scale = t.scale * 0.6f;
			t.position = Vector3(START_X + STEP_X * i, POS_Y, 0.0f);
			w.AddComponent<Transform>(id, t);
            
			w.AddComponent<SpriteRenderer>(id, sr);
		});
	}

	// ???????
	Push([this](ECS::World& w) {
		this->cursorId = w.CreateEntity();
		Transform t;
		t.position = Vector3(START_X, POS_Y, 0.0f);
		w.AddComponent<Transform>(this->cursorId, t);

		SpriteRenderer sr;
		sr.SetTexture("Assets/Texture/white.png");
		sr.size = Vector2{ STEP_X, 120.0f }; // ???????????
		sr.color = Vector4(1.0f, 1.0f, 1.0f, 0.5f); // ????????
		sr.isUI = true;
		sr.layer = SpriteLayer::UI_Back; // ???????
		w.AddComponent<SpriteRenderer>(this->cursorId, sr);
	});

	// SEロード
	cursorSE = LoadSound("Assets/Sound/UI/SFX_FastUiClickImpact08.wav");
	decideSE = LoadSound("Assets/Sound/UI/SFX_FastUiClickImpact08.wav");
}

void UpgradeScript::Update()
{
	// ??????
	if (IsKeyTrigger(VK_LEFT))
	{
		cursor--;
		if (cursor < 0) cursor = 2;
		Play(cursorSE);
	}
	if (IsKeyTrigger(VK_RIGHT))
	{
		cursor++;
		if (cursor > 2) cursor = 0;
		Play(cursorSE);
	}

	// ????????
	auto& t = world->GetComponent<Transform>(cursorId);
	t.position.x = START_X + STEP_X * cursor;

	if (IsKeyTrigger(VK_RETURN)) {
		Play(decideSE);
		PlayerManager::Upgrade((PlayerManager::Data)upgrades[cursor]);
		StageGenerator::Create();
	}
}

void UpgradeScript::OnDestroy()
{
}