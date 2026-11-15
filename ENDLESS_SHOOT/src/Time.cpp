#include "Time.h"

std::chrono::steady_clock::time_point Time::previousTime{};
float Time::deltaTime = 0.0f;
float Time::timeScale = 1.0f;

void Time::Reset()
{
    previousTime = std::chrono::steady_clock::now();
    deltaTime = 0.0f;
}

void Time::Update()
{
    const auto currentTime = std::chrono::steady_clock::now();

    const float elapsedSeconds = std::chrono::duration<float>(currentTime - previousTime).count();

    previousTime = currentTime;

    // Œo‰ßŽžŠÔ‚ªÅ‘å’l‚ð’´‚¦‚½ê‡‚ÍÅ‘å’l‚É§ŒÀ
    deltaTime = elapsedSeconds > MAX_DELTA_TIME ? MAX_DELTA_TIME : elapsedSeconds;
}
