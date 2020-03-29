#pragma once

#include "ray.h"

class Material;

struct hit_record {
	float t;
	Vector3 p;
	Vector3 normal;
	Material* mat;
};

class Surface {
public:
	virtual bool hit(const Ray& r, float tMin, float tMax, hit_record&rec) const = 0;
};
