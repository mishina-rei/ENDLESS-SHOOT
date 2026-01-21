#include "StageGenerator.h"
#include "SceneGame.h"
#include "SceneStage2.h"
#include "SceneStage3.h"
#include "SceneStage4.h"
#include "SceneStage5.h"
#include "SceneStage6.h"
#include "SceneStage7.h"
#include "SceneStage8.h"
#include "SceneStage9.h"
#include "SceneStage10.h"
#include "SceneStage11.h"
#include "SceneStage12.h"
#include "Random.h"
#include "SceneManager.h"

void StageGenerator::Create()
{
	// ランダムにステージの種類を決定
	// SceneGameは除く
	// ステージクラスが増えた場合は、Range(0, N)に変更しcaseを追加
	int type = Random::Range(0, 10);

	switch (type)
	{
	case 0:
        return SceneManager::GetInstance().SetScene<SceneStage2>();
	case 1:
        return SceneManager::GetInstance().SetScene<SceneStage3>();
	case 2:
        return SceneManager::GetInstance().SetScene<SceneStage4>();
	case 3:
        return SceneManager::GetInstance().SetScene<SceneStage5>();
	case 4:
        return SceneManager::GetInstance().SetScene<SceneStage6>();
	case 5:
        return SceneManager::GetInstance().SetScene<SceneStage7>();
	case 6:
		return SceneManager::GetInstance().SetScene<SceneStage8>();
	case 7:
		return SceneManager::GetInstance().SetScene<SceneStage9>();
	case 8:
		return SceneManager::GetInstance().SetScene<SceneStage10>();
	case 9:
		return SceneManager::GetInstance().SetScene<SceneStage11>();
	case 10:
		return SceneManager::GetInstance().SetScene<SceneStage12>();
	default:
		return SceneManager::GetInstance().SetScene<SceneGame>();
	}
}
