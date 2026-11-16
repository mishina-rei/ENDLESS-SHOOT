#include "PhysicsSystem.h"
#include "Rigidbody.h"
#include "Transform.h"
#include "Defines.h"
#include "Time.h"
#include <cmath>

void PhysicsSystem::Update(ECS::World* world)
{
    const float dt = Time::GetDeltaTime();
    if (dt <= 0.0f) return;

    Vector3 gravity = { 0.0f, -57.6f, 0.0f }; // 重力加速度

    world->ForEach<Rigidbody, Transform>([&](ECS::EntityID id, Rigidbody& rb, Transform& trans) {
        if (rb.isKinematic) return;

        // 重力の適用
        if (rb.useGravity) {
            rb.velocity += gravity * dt;
        }

        // 加速度の適用
        rb.velocity += rb.acceleration * dt;

        // 空気抵抗の適用
        if (rb.drag > 0.0f) {
            rb.velocity *= std::pow(rb.drag < 1.0f ? 1.0f - rb.drag : 0.0f, dt * fFPS);
        }

        // 速度による位置更新
        trans.position += rb.velocity * dt;
    });
}
