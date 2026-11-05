#include "Clear.h"
#include "EntityTag.h"
#include "GameManager.h"
#include "Component.h"
#include "Defines.h"
#include "Input.h"
#include "StageGenerator.h"
#include "SceneUpgrade.h"

Clear::Clear()
{
	clearDecideSE = LoadSound("Assets/Sound/UI/SFX_FastUiClickImpact08.wav");
}

void Clear::Update()
{
	if (!isClear)
	{
		if (GameManager::GetState() != GameManager::PLAY)
			return;

		// 敵が全滅しているか確認
		isClear = true;
		world->ForEachComponent<EnemyTag>([&](ECS::EntityID id, EnemyTag& tag)
			{
				isClear = false;
			});

		if (isClear)
		{
			// クリア演出開始
			GameManager::SetState(GameManager::CLEAR);

			Push([this](ECS::World& world)
				{
					// クリア表示エンティティの生成
					{
						ECS::EntityID id = world.CreateEntity();

						Transform t;
						t.position = Vector3(SCREEN_HALF_WIDTH, SCREEN_HALF_HEIGHT - 100.0f, 0.0f);
						world.AddComponent<Transform>(id, t);

						SpriteRenderer sr;
						sr.SetTexture("Assets/Texture/stageClear.png");
						sr.size = Vector2{ 824.0f, 65.0f };
						sr.isUI = true;
						sr.layer = SpriteLayer::Text; // 背景より手前
						world.AddComponent<SpriteRenderer>(id, sr);
					}

					// 暗転用背景 (半透明の黒)
					{
						ECS::EntityID bgId = world.CreateEntity();
						Transform t;
						t.position = Vector3((float)SCREEN_WIDTH * 0.5f, (float)SCREEN_HEIGHT * 0.5f, 0.0f);
						world.AddComponent<Transform>(bgId, t);

						SpriteRenderer sr;
						sr.SetTexture("Assets/Texture/white.png");
						sr.size = Vector2({ (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT });
						sr.color = Vector4(0.0f, 0.0f, 0.0f, 0.7f); // 黒で透明度0.7
						sr.isUI = true;
						sr.layer = SpriteLayer::Overlay; // UIより手前に表示
						world.AddComponent<SpriteRenderer>(bgId, sr);
					}

					// PressEnter テキスト
					{
						ECS::EntityID id = world.CreateEntity();

						Transform t;
						t.position = Vector3(SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f + 100.0f, 0.0f);
						t.scale = Vector3(0.9f, 0.9f, 1.0f);
						world.AddComponent<Transform>(id, t);

						SpriteRenderer sr;
						sr.SetTexture("Assets/Texture/pressEnter.png");
						sr.size = Vector2{ 621.0f, 71.0f };
						sr.layer = SpriteLayer::Text; // 背景より手前
						sr.isUI = true;
						world.AddComponent<SpriteRenderer>(id, sr);
					}
				});
		}
		return;
	}

	if (IsKeyTrigger(VK_RETURN))
	{
		Play(clearDecideSE);
		SceneManager::GetInstance().SetScene<SceneUpgrade>();
	}
}
