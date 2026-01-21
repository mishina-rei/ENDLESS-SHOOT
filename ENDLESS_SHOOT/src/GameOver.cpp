#include "GameOver.h"
#include "Input.h"
#include "SceneTitle.h"
#include "Component.h"
#include "GameManager.h"
#include <string>

void GameOver::OnCreate()
{
	// ゲームオーバー演出 (暗転 + テキスト)
	Push([](ECS::World& w) {
		// 1. 暗転用背景 (半透明の黒)
		{
			ECS::EntityID bgId = w.CreateEntity();
			Transform t;
			t.position = Vector3((float)SCREEN_WIDTH * 0.5f, (float)SCREEN_HEIGHT * 0.5f, 0.0f);
			w.AddComponent<Transform>(bgId, t);

			SpriteRenderer sr;
			sr.SetTexture("Assets/Texture/white.png");
			sr.size = Vector2({ (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT });
			sr.color = Vector4(0.0f, 0.0f, 0.0f, 0.7f); // 黒で透明度0.7
			sr.isUI = true;
			sr.layer = SpriteLayer::Overlay; // UIより手前に表示
			w.AddComponent<SpriteRenderer>(bgId, sr);
		}

		// 2. GAME OVER テキスト
		{
			ECS::EntityID textId = w.CreateEntity();
			Transform tt;
			tt.position = Vector3((float)SCREEN_WIDTH * 0.5f, (float)SCREEN_HEIGHT * 0.5f - 150.0f, 0.0f);
			tt.scale = Vector3(1.3f, 1.3f, 1.0f); // 文字を大きく
			w.AddComponent<Transform>(textId, tt);

			SpriteRenderer sr;
			sr.SetTexture("Assets/Texture/gameover.png");
			sr.size = Vector2({ 616.0f, 81.0f });
			sr.isUI = true;
			sr.color = Vector4(1.0f, 0.0f, 0.0f, 1.0f); // 赤文字
			sr.layer = SpriteLayer::Text; // 背景より手前
			w.AddComponent<SpriteRenderer>(textId, sr);
		}

		// PressEnter テキスト
		{
			ECS::EntityID id = w.CreateEntity();

			Transform t;
			t.position = Vector3(SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f + 150.0f, 0.0f);
			t.scale = Vector3(0.9f, 0.9f, 1.0f);
			w.AddComponent<Transform>(id, t);

			SpriteRenderer sr;
			sr.SetTexture("Assets/Texture/pressEnter.png");
			sr.size = Vector2{ 621.0f, 71.0f };
			sr.isUI = true;
			w.AddComponent<SpriteRenderer>(id, sr);
		}

		// ステージ数表示
		{
			int stage = GameManager::GetStageCount() - 1;
			std::string str = std::to_string(stage);
			float charWidth = 50.0f;
			float startX = (float)SCREEN_WIDTH * 0.5f - (str.length() * charWidth) * 0.5f + charWidth * 0.5f;

			for (size_t i = 0; i < str.length(); ++i)
			{
				ECS::EntityID id = w.CreateEntity();
				Transform t;
				t.position = Vector3(startX + i * charWidth, (float)SCREEN_HEIGHT * 0.5f, 0.0f);
				w.AddComponent<Transform>(id, t);

				SpriteRenderer sr;
				sr.SetTexture("Assets/Texture/num2.png");
				//sr.size = Vector2{ charWidth, 60.0f };
				sr.uvPos = Vector2{ (str[i] - '0') * 0.1f, 0.0f };
				sr.uvScale = Vector2{ 0.1f, 1.0f };
				sr.isUI = true;
				sr.layer = SpriteLayer::Text;
				w.AddComponent<SpriteRenderer>(id, sr);
			}

			// SCORE
			ECS::EntityID id = w.CreateEntity();
			Transform t;
			t.position = Vector3(startX - 160.0f, (float)SCREEN_HEIGHT * 0.5f, 0.0f);
			t.scale = Vector3(0.5f, 0.5f, 1.0f);
			w.AddComponent<Transform>(id, t);

			SpriteRenderer sr;
			sr.SetTexture("Assets/Texture/score.png");
			sr.isUI = true;
			sr.layer = SpriteLayer::Text;
			w.AddComponent<SpriteRenderer>(id, sr);
		}
		});

	// SEロード
	decideSE = LoadSound("Assets/Sound/UI/SFX_FastUiClickImpact08.wav");
}

void GameOver::Update()
{
	// エンターキーでタイトルへ遷移
	if (IsKeyTrigger(VK_RETURN))
	{
		Play(decideSE);
		SceneManager::GetInstance().SetScene<SceneTitle>();
	}
}
