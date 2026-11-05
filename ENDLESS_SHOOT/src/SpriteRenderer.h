#pragma once

#include "Texture.h"
#include "Vector.h"
#include <memory>

enum class SpriteLayer
{
	Default = 0,
	UI_Back,
	UI_Front,
	Overlay,
	Text,
};

struct SpriteRenderer
{
	std::shared_ptr<Texture> pTexture = nullptr;
	
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector2 size = { 100.0f, 100.0f };
	Vector2 pivot = { 0.5f, 0.5f }; // 0.5, 0.5 で中心基準
	
	// UVアニメーション用
	Vector2 uvPos = { 0.0f, 0.0f };
	Vector2 uvScale = { 1.0f, 1.0f };

	bool isVisible = true;
	bool isUI = false;
	SpriteLayer layer = SpriteLayer::Default;

	void SetTexture(const char* filePath)
	{
		pTexture = std::make_shared<Texture>();
		if (FAILED(pTexture->Create(filePath))) {
			MessageBox(NULL, "Texture load failed", "Error", MB_OK);
		} 

		size = Vector2({float(pTexture->GetWidth()), float(pTexture->GetHeight())});
	}
};