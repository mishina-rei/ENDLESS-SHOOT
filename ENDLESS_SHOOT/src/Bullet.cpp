#include "Bullet.h"
#include "Defines.h"
#include "Component.h"
#include "Gun.h"
#include "EffekseerManager.h"

void Bullet::Update()
{
	// Žõ–½‚ðŒ¸‚ç‚·
	lifeTime -= 1.0f / fFPS;
	if (lifeTime <= 0.0f)
	{
		ECS::EntityID id = this->entityId;
		Push([id](ECS::World& w) {
			w.DeleteEntity(id);
		});
	}
}

void Bullet::OnCollisionEnter(ECS::EntityID other)
{
    const auto& collider = world->GetComponent<BoxCollider>(other);
    if(collider.tag != CollisionTag::PLAYER && 
       collider.tag != CollisionTag::BULLET && 
       !collider.isTrigger)
    {
        // ?????????????????
        if (collider.tag == CollisionTag::ENEMY)
        {
            // ???????
            auto& t = GetComponent<Transform>();
            Effekseer::Handle handle = EffekseerManager::Play("Assets/Effect/spark.efkefc", t.position);
            EffekseerManager::SetScale(handle,Vector3 (0.1f,0.1f,0.1f));

            // ??????+Y?????????????????
            if (world->HasComponent<Rigidbody>(entityId))
            {
                auto& rb = GetComponent<Rigidbody>();
                Vector3 vel = rb.velocity;
                if (vel.MagnitudeSq() > 0.0001f)
                {
                    Vector3 dir = -vel.Normalized();
                    float ry = atan2f(dir.x, dir.z);
                    float y = dir.y;
                    if (y > 1.0f) y = 1.0f;
                    if (y < -1.0f) y = -1.0f;
                    float rx = acosf(y);
                    EffekseerManager::SetRotation(handle, Vector3(rx, ry, 0.0f));
                }
            }

            if (world->HasComponent<Script>(gunId))
            {
                auto& scriptComp = world->GetComponent<Script>(gunId);
                if (scriptComp.script)
                {
                    Gun* gun = dynamic_cast<Gun*>(scriptComp.script.get());
                    if (gun) {
                        gun->ShowHitMarker();
                    }
                }
            }
        }

        ECS::EntityID id = this->entityId;
        Push([id](ECS::World& w) {
            w.DeleteEntity(id);
        });
    }
}