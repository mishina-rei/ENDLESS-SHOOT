#pragma once

#include "Vector.h"
#include "Quaternion.h"

struct Transform
{
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;

	Transform()
		: position(0.0f, 0.0f, 0.0f)
		, rotation(Quaternion::Identity())
		, scale(1.0f, 1.0f, 1.0f)
	{}

	void Rotate(Quaternion q)
	{
		rotation *= q;
		rotation = rotation.Normalize();
	}

	void Translate(const Vector3& v)
	{
		position += v;
	}
};