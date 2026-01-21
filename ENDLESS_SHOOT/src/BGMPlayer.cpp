#include "BGMPlayer.h"
#include "Audio.h"

BGMPlayer::BGMPlayer(const char* filename, float volume)
	: m_filename(filename)
	, m_volume(volume)
	, m_handle(INVALID_SOUND_HANDLE)
{
}

BGMPlayer::BGMPlayer(float volume)
	: m_filename("Assets/Sound/BGM/bgm20260116.wav")
	, m_volume(volume)
	, m_handle(INVALID_SOUND_HANDLE)
{
}

BGMPlayer::~BGMPlayer()
{
	// クラス破棄時にBGMを停止
	if (m_handle != INVALID_SOUND_HANDLE)
	{
		StopSound(m_handle);
	}
}

void BGMPlayer::OnCreate()
{
	// BGMロードと再生（ループ再生）
	m_handle = LoadSound(m_filename);
	if (m_handle != INVALID_SOUND_HANDLE)
	{
		SetVolume(m_handle, m_volume);
		Play(m_handle, true);
	}
}
