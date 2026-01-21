#pragma once

#include "Model.h"
#include "Vector.h"
#include <memory>

struct MeshRenderer
{
	std::shared_ptr<Model> pModel = nullptr;
	bool isVisible = true;
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

	MeshRenderer()
	{
		pModel = std::make_shared<Model>();
	}
	MeshRenderer(Model* _pModel, bool visible = true)
		: pModel(_pModel), isVisible(visible) // ???|?C???^????L??????????
	{
	}
};