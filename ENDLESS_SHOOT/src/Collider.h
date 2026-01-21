#pragma once

#include "Vector.h"
#include "Quaternion.h"
#include "EntityID.h"
#include <string>

// 衝突判定用のタグ定義
enum class CollisionTag
{
    Default,
    NONE,
    PLAYER,
    ENEMY,
    BULLET,

    // 必要に応じて追加
    COLLISION_TAG_MAX
};

struct BoxCollider
{
	Vector3 center = { 0.0f, 0.0f, 0.0f };	    // ???[?J?????W???I?t?Z?b?g
	Vector3 size = { 1.0f, 1.0f, 1.0f };	    // Box?????T?C?Y
    Quaternion rotation = Quaternion::Identity();	// ???[?J?????W?????]
	bool isTrigger = false;					    // ?????????????C?x???g????????????
    bool isStatic = true;                       // ??I?????I??
    CollisionTag tag = CollisionTag::NONE;   // 識別用タグ

    BoxCollider(){}

    BoxCollider(Vector3 _center, Vector3 _size, Quaternion _rotation, bool _isTrigger = false, CollisionTag _tag = CollisionTag::Default):
        center(_center), size(_size), rotation(_rotation), isTrigger(_isTrigger), tag(_tag)
    {
    }

    void Rotate(Quaternion q)
	{
		rotation *= q;
		rotation = rotation.Normalize();
	}
};

// ?????????v?Z?p?\????
struct Axis3
{
    Vector3 x;
    Vector3 y;
    Vector3 z;
};

struct ObbData
{
    Vector3 pos;
    Vector3 scale;
    Axis3 axis;
    ECS::EntityID entityId;
    CollisionTag tag;
};