#pragma once
#include "world.h"
#include <DirectXMath.h>

class RenderSystem
{
public:
	// 描画実行
	// view, proj: カメラのビュー・プロジェクション行列
	static void Draw(ECS::World* world);
};