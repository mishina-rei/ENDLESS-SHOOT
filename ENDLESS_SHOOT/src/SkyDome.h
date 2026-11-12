#pragma once

#include "Model.h"
#include "Vector.h"
#include <memory>

struct SkyDome
{
	std::shared_ptr<Model> pModel = nullptr;	// モデルデータ
	bool isVisible = true;						// 表示するかどうか

	SkyDome()
	{
		pModel = std::make_shared<Model>();
	}
	SkyDome(Model* _pModel, bool visible = true)
		: pModel(_pModel), isVisible(visible)
	{
	}
	SkyDome(std::shared_ptr<Model> _pModel, bool visible = true)
		: pModel(_pModel), isVisible(visible)
	{
	}
};
