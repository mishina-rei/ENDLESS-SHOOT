#pragma once

#include <vector>

class PlayerManager
{
public:

    enum Data
    {
        MAX_HP,
        ATTACK_POWER,
        FIRE_RATE,
        MAGAZINE_SIZE,

        DATA_MAX
    };

	// 初期化
	static void Init();

	// 最大HP (強化データ)
	static float GetMaxHp();

	// 攻撃力 (強化データ)
	static float GetAttackPower();

	// 連射速度 (強化データ)
	static float GetFireRate();

	// マガジンサイズ (強化データ)
	static float GetMagazineSize();

	// 強化レベルアップ
	static void Upgrade(Data type);

private:

    static std::vector<int> data;   // 強化データ配列
};