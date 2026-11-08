#include "StageBackground.h"
#include "world.h"
#include "Component.h"
#include "ShaderList.h"
#include <memory>

void StageBackground::Create(ECS::World& world)
{
// --- ステージ外周のビル・壁モデル配置 ---
{
	// モデルのロードヘルパー（shared_ptrで各モデルを1度だけロードして使い回す）
	auto LoadModel = [](const char* p) {
		auto model = std::make_shared<Model>();
		model->Load(p);
		model->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
		model->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
		return model;
	};

	// 背景用超高層ビル（3種類）
	auto pBuilding01 = LoadModel("Assets/Model/building/SM_BG_Building01.FBX");
	auto pBuilding02 = LoadModel("Assets/Model/building/SM_BG_Building02.FBX");
	auto pBuilding03 = LoadModel("Assets/Model/building/SM_BG_Building03.FBX");

	// 手前・中層の壁・タワー・ピラー
	auto pTower      = LoadModel("Assets/Model/building/Tower8x.fbx");
	auto pWall       = LoadModel("Assets/Model/building/Wall6x_01.fbx");
	auto pMid        = LoadModel("Assets/Model/building/SM_BuildMid01_low.FBX");
	auto pSkyTower1  = LoadModel("Assets/Model/building/SkyTower1.fbx");
	auto pSkyTower2  = LoadModel("Assets/Model/building/SkyTower2.fbx");
	auto pSkyTower3  = LoadModel("Assets/Model/building/SkyTower3.fbx");

	// 配置ヘルパー（ Transform + MeshRenderer のみ）
	auto PlaceProp = [&](std::shared_ptr<Model> model, const Vector3& pos, float yawDeg, const Vector3& scale = Vector3(1.0f, 1.0f, 1.0f)) {
		ECS::EntityID entity = world.CreateEntity();
		Transform t;
		t.position = pos;
		t.rotation = Quaternion::FromRotation(0.0f, yawDeg, 0.0f);
		t.scale = scale;
		world.AddComponent<Transform>(entity, t);
		world.AddComponent<MeshRenderer>(entity, MeshRenderer(model));
		return entity;
	};

	// ---------------------------------------------------------
	// 最遠景のビル群
	// ---------------------------------------------------------
	//(+Z)
	PlaceProp(pBuilding02, Vector3(-18.0f, 0.0f, 40.0f),   0.0f);
	PlaceProp(pBuilding03, Vector3(  0.0f, 0.0f, 42.0f),  45.0f);
	PlaceProp(pBuilding01, Vector3( 18.0f, 0.0f, 39.0f),  90.0f);
	PlaceProp(pBuilding01, Vector3(-28.0f, 0.0f, 58.0f), 180.0f);
	PlaceProp(pBuilding03, Vector3(-10.0f, 0.0f, 60.0f),   0.0f);
	PlaceProp(pBuilding02, Vector3( 10.0f, 0.0f, 57.0f),   0.0f);
	PlaceProp(pBuilding01, Vector3( 28.0f, 0.0f, 59.0f), 270.0f);

	// (-Z)
	PlaceProp(pBuilding01, Vector3(-18.0f, 0.0f, -39.0f),   0.0f);
	PlaceProp(pBuilding02, Vector3(  0.0f, 0.0f, -40.0f), 180.0f);
	PlaceProp(pBuilding03, Vector3( 18.0f, 0.0f, -42.0f),  45.0f);
	PlaceProp(pBuilding02, Vector3(-26.0f, 0.0f, -57.0f), 180.0f);
	PlaceProp(pBuilding01, Vector3( -8.0f, 0.0f, -59.0f), 270.0f);
	PlaceProp(pBuilding03, Vector3( 11.0f, 0.0f, -60.0f),   0.0f);
	PlaceProp(pBuilding01, Vector3( 28.0f, 0.0f, -58.0f), 180.0f);

	// (+X)
	PlaceProp(pBuilding03, Vector3( 42.0f, 0.0f, -18.0f),   0.0f);
	PlaceProp(pBuilding01, Vector3( 39.0f, 0.0f,   0.0f), 270.0f);
	PlaceProp(pBuilding02, Vector3( 40.0f, 0.0f,  18.0f),  90.0f);
	PlaceProp(pBuilding01, Vector3( 59.0f, 0.0f, -28.0f),  90.0f);
	PlaceProp(pBuilding02, Vector3( 57.0f, 0.0f, -10.0f),  90.0f);
	PlaceProp(pBuilding03, Vector3( 60.0f, 0.0f,  10.0f),  45.0f);
	PlaceProp(pBuilding01, Vector3( 58.0f, 0.0f,  28.0f), 180.0f);

	// (-X)
	PlaceProp(pBuilding02, Vector3(-40.0f, 0.0f, -18.0f),  90.0f);
	PlaceProp(pBuilding03, Vector3(-42.0f, 0.0f,   0.0f),  45.0f);
	PlaceProp(pBuilding01, Vector3(-39.0f, 0.0f,  18.0f),  90.0f);
	PlaceProp(pBuilding03, Vector3(-60.0f, 0.0f, -28.0f),   0.0f);
	PlaceProp(pBuilding01, Vector3(-58.0f, 0.0f,  -9.0f),   0.0f);
	PlaceProp(pBuilding02, Vector3(-57.0f, 0.0f,  11.0f),  90.0f);
	PlaceProp(pBuilding01, Vector3(-59.0f, 0.0f,  28.0f), 270.0f);

	// 四隅
	PlaceProp(pBuilding03, Vector3( 48.0f, 0.0f,  48.0f),  45.0f);
	PlaceProp(pBuilding02, Vector3(-48.0f, 0.0f,  48.0f),  45.0f);
	PlaceProp(pBuilding02, Vector3( 48.0f, 0.0f, -48.0f), 135.0f);
	PlaceProp(pBuilding03, Vector3(-48.0f, 0.0f, -48.0f),  45.0f);

	// ---------------------------------------------------------
	// 手前外周（第1層: 境界 ±20mライン / 第2層: 奥行き中層ライン ±24m）
	// ---------------------------------------------------------
	// 基準オフセット（手前表面をほぼ ±20.0f に合わせつつ、微小オフセットで面競合を防止）
	const float WALL_Z   = 20.0f + 1.13f;  // 21.13f
	const float TOWER_Z  = 20.0f + 2.28f;  // 22.28f
	const float MID_Z    = 20.0f + 0.00f;  // 20.00f
	const float SKY12_Z  = 20.0f + 1.50f;  // 21.50f
	const float SKY3_Z   = 20.0f + 1.00f;  // 21.00f

	// (+Z)
	// --- 第1層（手前） ---
	// 中央ゲート壁（2段重ね: 高さ8m）
	PlaceProp(pWall,      Vector3(  0.0f, 0.0f, WALL_Z + 0.04f), 180.0f);
	PlaceProp(pMid,       Vector3(  0.0f, 4.0f,  MID_Z + 0.02f), 180.0f);

	// ゲート両脇の高層ピラー
	PlaceProp(pSkyTower3, Vector3( -3.8f, 0.0f, SKY3_Z + 0.06f),   0.0f);
	PlaceProp(pSkyTower3, Vector3( -3.8f, 4.0f, SKY3_Z + 0.06f),   0.0f);
	PlaceProp(pSkyTower3, Vector3( -3.8f, 8.0f, SKY3_Z + 0.06f),   0.0f);
	PlaceProp(pSkyTower3, Vector3(  3.8f, 0.0f, SKY3_Z + 0.06f),   0.0f);
	PlaceProp(pSkyTower3, Vector3(  3.8f, 4.0f, SKY3_Z + 0.06f),   0.0f);
	PlaceProp(pSkyTower3, Vector3(  3.8f, 8.0f, SKY3_Z + 0.06f),   0.0f);

	// 中間防壁
	PlaceProp(pWall,      Vector3( -7.6f, 0.0f, WALL_Z + 0.02f), 180.0f);
	PlaceProp(pWall,      Vector3(  7.6f, 0.0f, WALL_Z + 0.02f), 180.0f);
	PlaceProp(pWall,      Vector3(  7.6f, 4.0f, WALL_Z + 0.06f), 180.0f); // 右側のみ2段重ね

	
	PlaceProp(pSkyTower1, Vector3(-12.0f, 0.0f, SKY12_Z + 0.03f),  90.0f);
	PlaceProp(pSkyTower2, Vector3(-12.0f, 4.0f, SKY12_Z + 0.05f),  90.0f);
	PlaceProp(pSkyTower2, Vector3( 12.0f, 0.0f, SKY12_Z + 0.03f),  90.0f);
	PlaceProp(pSkyTower1, Vector3( 12.0f, 4.0f, SKY12_Z + 0.05f),  90.0f);

	PlaceProp(pMid,       Vector3(-16.6f, 0.0f,  MID_Z + 0.03f), 180.0f);
	PlaceProp(pMid,       Vector3( 16.6f, 0.0f,  MID_Z + 0.03f), 180.0f);

	// --- 第2層（奥） ---
	PlaceProp(pTower,     Vector3(  0.0f, 0.0f, 24.5f), 180.0f);

	PlaceProp(pTower,     Vector3(-10.5f, 0.0f, 25.0f), 180.0f);
	PlaceProp(pTower,     Vector3( 10.5f, 0.0f, 25.0f), 180.0f);
	PlaceProp(pSkyTower3, Vector3(-16.0f, 0.0f, 24.0f),   0.0f);
	PlaceProp(pSkyTower3, Vector3(-16.0f, 4.0f, 24.0f),   0.0f);
	PlaceProp(pSkyTower3, Vector3( 16.0f, 0.0f, 24.0f),   0.0f);
	PlaceProp(pSkyTower3, Vector3( 16.0f, 4.0f, 24.0f),   0.0f);

	// 南辺 (-Z)
	// --- 第1層（手前） ---
	PlaceProp(pMid,       Vector3(  0.0f, 0.0f, -(MID_Z + 0.02f)),   0.0f);

	PlaceProp(pSkyTower3, Vector3( -3.8f, 0.0f, -(SKY3_Z + 0.05f)),   0.0f);
	PlaceProp(pSkyTower3, Vector3( -3.8f, 4.0f, -(SKY3_Z + 0.05f)),   0.0f);
	PlaceProp(pSkyTower3, Vector3(  3.8f, 0.0f, -(SKY3_Z + 0.05f)),   0.0f);
	PlaceProp(pSkyTower3, Vector3(  3.8f, 4.0f, -(SKY3_Z + 0.05f)),   0.0f);

	PlaceProp(pWall,      Vector3( -7.8f, 0.0f, -(WALL_Z + 0.03f)),   0.0f);
	PlaceProp(pWall,      Vector3( -7.8f, 4.0f, -(WALL_Z + 0.06f)),   0.0f); 
	PlaceProp(pWall,      Vector3(  7.8f, 0.0f, -(WALL_Z + 0.02f)),   0.0f);

	PlaceProp(pSkyTower1, Vector3(-12.2f, 0.0f, -(SKY12_Z + 0.04f)), 270.0f);
	PlaceProp(pSkyTower1, Vector3(-12.2f, 4.0f, -(SKY12_Z + 0.07f)), 270.0f); // 2段重ね
	PlaceProp(pSkyTower2, Vector3( 12.2f, 0.0f, -(SKY12_Z + 0.03f)), 270.0f);

	// 外側
	PlaceProp(pWall,      Vector3(-16.6f, 0.0f, -(WALL_Z + 0.02f)),   0.0f);
	PlaceProp(pWall,      Vector3( 16.6f, 0.0f, -(WALL_Z + 0.04f)),   0.0f);

	// --- 第2層（奥） ---
	PlaceProp(pTower,     Vector3( -5.5f, 0.0f, -24.5f),   0.0f);
	PlaceProp(pTower,     Vector3(  5.5f, 0.0f, -24.5f),   0.0f);
	PlaceProp(pMid,       Vector3(-14.0f, 0.0f, -24.0f),   0.0f);
	PlaceProp(pMid,       Vector3(-14.0f, 4.0f, -24.0f),   0.0f);
	PlaceProp(pMid,       Vector3( 14.0f, 0.0f, -24.0f),   0.0f);
	PlaceProp(pMid,       Vector3( 14.0f, 4.0f, -24.0f),   0.0f);

	// (+X)
	// --- 第1層（手前） ---
	// メインタワー（3段重ね: 高さ12m）
	PlaceProp(pSkyTower1, Vector3(SKY12_Z + 0.05f, 0.0f,   0.0f), 180.0f);
	PlaceProp(pSkyTower1, Vector3(SKY12_Z + 0.05f, 4.0f,   0.0f), 180.0f);
	PlaceProp(pSkyTower1, Vector3(SKY12_Z + 0.05f, 8.0f,   0.0f), 180.0f);

	// タワー脇
	PlaceProp(pSkyTower3, Vector3(SKY3_Z + 0.06f, 0.0f,  -2.6f),   0.0f);
	PlaceProp(pSkyTower3, Vector3(SKY3_Z + 0.06f, 0.0f,   2.6f),   0.0f);

	//（右側2段重ね）
	PlaceProp(pWall,      Vector3(WALL_Z + 0.03f, 0.0f,  -6.8f), 270.0f);
	PlaceProp(pWall,      Vector3(WALL_Z + 0.02f, 0.0f,   6.8f), 270.0f);
	PlaceProp(pWall,      Vector3(WALL_Z + 0.06f, 4.0f,   6.8f), 270.0f);

	// 2段重ね: 高さ8m
	PlaceProp(pMid,       Vector3(MID_Z + 0.02f, 0.0f, -11.8f), 270.0f);
	PlaceProp(pMid,       Vector3(MID_Z + 0.05f, 4.0f, -11.8f), 270.0f);
	PlaceProp(pMid,       Vector3(MID_Z + 0.02f, 0.0f,  11.8f), 270.0f);
	PlaceProp(pMid,       Vector3(MID_Z + 0.05f, 4.0f,  11.8f), 270.0f);

	// 外側タワー
	PlaceProp(pSkyTower2, Vector3(SKY12_Z + 0.03f, 0.0f, -16.6f), 180.0f);
	PlaceProp(pTower,     Vector3(TOWER_Z + 0.04f, 0.0f,  16.6f), 270.0f);

	// --- 第2層（奥） ---
	PlaceProp(pTower,     Vector3(24.5f, 0.0f,   0.0f), 270.0f);
	PlaceProp(pSkyTower3, Vector3(24.0f, 0.0f,  -8.0f),   0.0f);
	PlaceProp(pSkyTower3, Vector3(24.0f, 4.0f,  -8.0f),   0.0f);
	PlaceProp(pSkyTower3, Vector3(24.0f, 0.0f,   8.0f),   0.0f);
	PlaceProp(pSkyTower3, Vector3(24.0f, 4.0f,   8.0f),   0.0f);
	PlaceProp(pWall,      Vector3(24.2f, 0.0f, -14.5f), 270.0f);
	PlaceProp(pWall,      Vector3(24.2f, 4.0f, -14.5f), 270.0f);
	PlaceProp(pWall,      Vector3(24.2f, 0.0f,  14.5f), 270.0f);

	// (-X)
	// --- 第1層（手前） ---
	// 中央（2段重ね: 高さ8m）
	PlaceProp(pMid,       Vector3(-(MID_Z + 0.02f), 0.0f,   0.0f),  90.0f);
	PlaceProp(pMid,       Vector3(-(MID_Z + 0.05f), 4.0f,   0.0f),  90.0f);

	// 左右タワー（2段重ね: 高さ8m）
	PlaceProp(pSkyTower2, Vector3(-(SKY12_Z + 0.03f), 0.0f,  -4.5f),   0.0f);
	PlaceProp(pSkyTower2, Vector3(-(SKY12_Z + 0.06f), 4.0f,  -4.5f),   0.0f);
	PlaceProp(pSkyTower1, Vector3(-(SKY12_Z + 0.03f), 0.0f,   4.5f),   0.0f);
	PlaceProp(pSkyTower1, Vector3(-(SKY12_Z + 0.06f), 4.0f,   4.5f),   0.0f);

	// 防壁
	PlaceProp(pWall,      Vector3(-(WALL_Z + 0.03f), 0.0f,  -8.8f),  90.0f);
	PlaceProp(pWall,      Vector3(-(WALL_Z + 0.06f), 4.0f,  -8.8f),  90.0f); // 2段重ね
	PlaceProp(pWall,      Vector3(-(WALL_Z + 0.02f), 0.0f,   8.8f),  90.0f);

	//（3段重ね: 高さ12m）
	PlaceProp(pTower,     Vector3(-(TOWER_Z + 0.04f), 0.0f, -13.5f),  90.0f);
	PlaceProp(pSkyTower3, Vector3(-(SKY3_Z + 0.05f),  0.0f,  13.5f),   0.0f);
	PlaceProp(pSkyTower3, Vector3(-(SKY3_Z + 0.05f),  4.0f,  13.5f),   0.0f);
	PlaceProp(pSkyTower3, Vector3(-(SKY3_Z + 0.05f),  8.0f,  13.5f),   0.0f);

	// 外側
	PlaceProp(pWall,      Vector3(-(WALL_Z + 0.02f), 0.0f, -17.0f),  90.0f);
	PlaceProp(pWall,      Vector3(-(WALL_Z + 0.04f), 0.0f,  17.0f),  90.0f);

	// --- 第2層（奥） ---
	PlaceProp(pTower,     Vector3(-24.5f, 0.0f,  -6.5f),  90.0f);
	PlaceProp(pTower,     Vector3(-24.5f, 0.0f,   6.5f),  90.0f);
	PlaceProp(pSkyTower3, Vector3(-24.0f, 0.0f,   0.0f),   0.0f);
	PlaceProp(pSkyTower3, Vector3(-24.0f, 4.0f,   0.0f),   0.0f);
	PlaceProp(pMid,       Vector3(-24.0f, 0.0f, -14.5f),  90.0f);
	PlaceProp(pMid,       Vector3(-24.0f, 4.0f, -14.5f),  90.0f);
	PlaceProp(pMid,       Vector3(-24.0f, 0.0f,  14.5f),  90.0f);
	PlaceProp(pMid,       Vector3(-24.0f, 4.0f,  14.5f),  90.0f);

	// 四隅
	const float CORNER_OFFSET = 20.0f + 1.61f; // 21.61f

	// (X),(Z)
	PlaceProp(pTower,     Vector3( CORNER_OFFSET, 0.0f,  CORNER_OFFSET), 225.0f);
	PlaceProp(pSkyTower3, Vector3( 24.5f, 0.0f,  24.5f),   0.0f);
	PlaceProp(pSkyTower3, Vector3( 24.5f, 4.0f,  24.5f),   0.0f);
	PlaceProp(pSkyTower3, Vector3( 24.5f, 8.0f,  24.5f),   0.0f);

	// (-X),(Z)
	PlaceProp(pSkyTower1, Vector3(-SKY12_Z, 0.0f, SKY12_Z), 135.0f);
	PlaceProp(pSkyTower1, Vector3(-SKY12_Z, 4.0f, SKY12_Z), 135.0f);
	PlaceProp(pTower,     Vector3(-24.8f,  0.0f,  24.8f), 135.0f);

	// (X),(-Z)
	PlaceProp(pSkyTower2, Vector3( SKY12_Z, 0.0f, -SKY12_Z), 315.0f);
	PlaceProp(pSkyTower2, Vector3( SKY12_Z, 4.0f, -SKY12_Z), 315.0f);
	PlaceProp(pTower,     Vector3( 24.8f,  0.0f, -24.8f), 315.0f);

	// (-X),(-Z)
	PlaceProp(pTower,     Vector3(-CORNER_OFFSET, 0.0f, -CORNER_OFFSET),  45.0f);
	PlaceProp(pSkyTower3, Vector3(-24.5f, 0.0f, -24.5f),   0.0f);
	PlaceProp(pSkyTower3, Vector3(-24.5f, 4.0f, -24.5f),   0.0f);
	PlaceProp(pSkyTower3, Vector3(-24.5f, 8.0f, -24.5f),   0.0f);
}
}
