#pragma once

#include "EntityID.h"
#include "Vector.h"

namespace ECS
{
    class World;
}

namespace StageObstacle
{
    enum class Type
    {
        Auto,
        Wall,
        Pillar,
        Block,
        Cover
    };

    // center and size describe the existing collider in world units.
    ECS::EntityID Create(ECS::World& world, const Vector3& center,
        const Vector3& size, Type type = Type::Auto);
}
