#include "EnemyBullet.h"
#include "Defines.h"
#include "Component.h"
#include "EntityTag.h"
#include "Player.h"
#include "GameManager.h"

void EnemyBullet::Update()
{
	// 寿命を減らす
	lifeTime -= 1.0f / fFPS;
	if (lifeTime <= 0.0f)
	{
		ECS::EntityID id = this->entityId;
		Push([id](ECS::World& w) {
			w.DeleteEntity(id);
		});
	}
}

void EnemyBullet::OnCollisionEnter(ECS::EntityID other)
{
	// プレイヤーに当たったらダメージを与えて消滅
	if (world->HasComponent<PlayerTag>(other))
	{
		if (world->HasComponent<Script>(other))
		{
			auto& scriptComp = world->GetComponent<Script>(other);
			if (scriptComp.script)
			{
				// Playerスクリプトを取得してダメージを与える
				// Shockwave.cppの実装を参考にキャスト
				Player* player = dynamic_cast<Player*>(scriptComp.script.get());
				if (player)
				{
					float currentDamage = damage + (GameManager::GetStageCount() - 1) * 0.2f;
					player->Damage(currentDamage);
				}
			}
		}

		// 弾を削除
		ECS::EntityID id = this->entityId;
		Push([id](ECS::World& w) {
			w.DeleteEntity(id);
		});
		return;
	}

	// 敵自身や他の弾、トリガー（Shockwaveなど）は無視する
	if (world->HasComponent<BoxCollider>(other))
	{
		auto& collider = world->GetComponent<BoxCollider>(other);
		if (collider.tag == CollisionTag::ENEMY ||
			collider.tag == CollisionTag::BULLET ||
			collider.isTrigger)
		{
			return;
		}
	}

	// 壁などに当たったら消滅
	ECS::EntityID id = this->entityId;
	Push([id](ECS::World& w) {
		w.DeleteEntity(id);
	});
}
