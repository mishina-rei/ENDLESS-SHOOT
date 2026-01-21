#pragma once
#include "world.h"
#include "Vector.h"

class EnemyGenerator
{
public:
	static void Create(ECS::World& world, const Vector3& position);
};
