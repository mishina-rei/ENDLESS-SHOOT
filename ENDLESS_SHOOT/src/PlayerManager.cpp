#include "PlayerManager.h"

std::vector<int> PlayerManager::data;

void PlayerManager::Init()
{
    // ƒŠƒZƒbƒg
    data.clear();
    data.resize(DATA_MAX);
    
    for(int i = 0; i < DATA_MAX; i++)
    {
        data[i] = 0;
    }
}

float PlayerManager::GetMaxHp()
{
	return 10.0f + (data[MAX_HP] * 2.0f);
}

float PlayerManager::GetAttackPower()
{
	return 1.0f + (data[ATTACK_POWER] * 0.5f);
}

float PlayerManager::GetFireRate()
{
	float val = 0.3f - (data[FIRE_RATE] * 0.02f);
	if (val < 0.05f) val = 0.05f;
	return val;
}

float PlayerManager::GetMagazineSize()
{
	return 30.0f + (data[MAGAZINE_SIZE] * 5.0f);
}

void PlayerManager::Upgrade(Data type)
{
	if (type < 0 || type >= DATA_MAX) return;
	if (data.size() < DATA_MAX)
	{
		data.resize(DATA_MAX, 0);
	}
	data[type]++;
}