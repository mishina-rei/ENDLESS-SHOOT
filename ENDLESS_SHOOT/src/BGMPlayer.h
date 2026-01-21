#pragma once
#include "NativeScript.h"

class BGMPlayer : public NativeScript
{
public:
	// コンストラクタでファイル名と音量を指定
	BGMPlayer(const char* filename, float volume = 0.5f);
	BGMPlayer(float volume);
	~BGMPlayer();

	void OnCreate() override;

private:
	const char* m_filename;
	float m_volume;
	int m_handle;
};
