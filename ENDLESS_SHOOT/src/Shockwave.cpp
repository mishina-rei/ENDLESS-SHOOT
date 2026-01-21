#include "Shockwave.h"
#include "Defines.h"
#include "Component.h"
#include "EntityTag.h"
#include "Player.h"
#include "GameManager.h"

static constexpr float DAMAGE = 0.5f;

void Shockwave::Update()
{
	// ?
	lifeTime -= 1.0f / fFPS;
	if (lifeTime <= 0.0f)
	{
		ECS::EntityID id = this->entityId;
		Push([id](ECS::World& w) {
			w.DeleteEntity(id);
		});
	}
}

void Shockwave::OnCollisionEnter(ECS::EntityID other)
{
    if(world->HasComponent<PlayerTag>(other))
    {
         float currentDamage = DAMAGE + (GameManager::GetStageCount() - 1) * 0.1f;
         static_cast<Player*>(world->GetComponent<Script>(other).script.get())->Damage(currentDamage);
    }
}