#pragma once

#include "Model.h"
#include "Vector.h"
#include <memory>

struct MeshRenderer
{
	std::shared_ptr<Model> pModel = nullptr;	// モデルデータ
	bool isVisible = true;						// 表示するかどうか
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };	// 色

	MeshRenderer()
	{
		pModel = std::make_shared<Model>();
	}
	MeshRenderer(Model* _pModel, bool visible = true)
		: pModel(_pModel), isVisible(visible)
	{
	}
	MeshRenderer(std::shared_ptr<Model> _pModel, bool visible = true)
		: pModel(_pModel), isVisible(visible)
	{
	}
};