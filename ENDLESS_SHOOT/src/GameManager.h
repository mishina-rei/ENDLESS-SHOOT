/*****************************************************************//**
 * @file   GameManger.h
 * @brief  何ステージ進んだか記録する
 *
 * @author
 * @date
 *********************************************************************/
#pragma once

class GameManager
{
public:

	enum State
	{
		PLAY,
		GAMEOVER,
		CLEAR
	};
	// ステージ数を進める
	static void AdvanceStage() { stageCount++; }

	// 現在のステージ数を取得
	static int GetStageCount() { return stageCount; }

	// ステージ数をリセット
	static void ResetStage() { stageCount = 0; }

	static State GetState() { return gameState; }
	static void SetState(State state) { gameState = state; }

private:

	static int stageCount;
	static State gameState;
};