#include "SceneTitle.h"
#include "Component.h"
#include "TitleScript.h"
#include "Defines.h"

void SceneTitle::Init()
{
	// 構造的コンポーネントの登録
	world.RegisterStructural<Transform>();
	world.RegisterStructural<SpriteRenderer>();

	// タイトル画像の作成 (UI)
	{
		ECS::EntityID id = world.CreateEntity();

		Transform t;
		t.position = Vector3(SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f - 100.0f, 0.0f);
		t.scale = Vector3(0.5f, 0.5f, 1.0f);
		world.AddComponent<Transform>(id, t);

		SpriteRenderer sr;
		// タイトル用の画像をセット
		sr.SetTexture("Assets/Texture/title.png");
		sr.isUI = true;
		world.AddComponent<SpriteRenderer>(id, sr);

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
	}

	// 制御用スクリプトを持つエンティティの作成
	{
		ECS::EntityID id = world.CreateEntity();
		Script script;
		script.Bind<TitleScript>();
		world.AddComponent<Script>(id, script);
	}
}