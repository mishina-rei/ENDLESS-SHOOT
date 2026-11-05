#include "SceneUpgrade.h"
#include "UpgradeScript.h"
#include "Component.h"
#include "Defines.h"

void SceneUpgrade::Init()
{
	world.RegisterStructural<Transform>();
	world.RegisterStructural<SpriteRenderer>();

	// 強化画面制御用スクリプト
	ECS::EntityID id = world.CreateEntity();
	Script script;
	script.Bind<UpgradeScript>();
	world.AddComponent<Script>(id, script);
}
