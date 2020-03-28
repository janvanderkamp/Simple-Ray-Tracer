#pragma once

#include "ray.h"

struct hit_record {
	float t;
	Vector3 p;
	Vector3 normal;
};

class Surface {
public:
	virtual bool hit(const Ray& r, float tMin, float tMax, hit_record&rec) const = 0;
};
