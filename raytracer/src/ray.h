#pragma once
#include "vector3.h"

class Ray
{
public:
	Ray() {}
	Ray(const Vector3& a, const Vector3 &b) : O(a), D(b) {}

	inline const Vector3& origin() const { return O; }
	inline const Vector3& direction() const { return D; }
	inline Vector3 pointAtParameter(float t)  const { return O + D * t; } 

	Vector3 O;
	Vector3 D;
};
