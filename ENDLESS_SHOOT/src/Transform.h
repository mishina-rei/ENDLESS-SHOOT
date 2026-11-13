#pragma once

#include "Vector.h"
#include "Quaternion.h"

struct Transform
{
	Vector3 position;		// ç¿ïW
	Quaternion rotation;	// âÒì]
	Vector3 scale;			// ägèk

	Transform()
		: position(0.0f, 0.0f, 0.0f)
		, rotation(Quaternion::Identity())
		, scale(1.0f, 1.0f, 1.0f)
	{}

	// âÒì]Ç≥ÇπÇÈ
	void Rotate(Quaternion q)
	{
		rotation *= q;
		rotation = rotation.Normalize();
	}

	// à⁄ìÆÇ≥ÇπÇÈ
	void Translate(const Vector3& v)
	{
		position += v;
	}
};