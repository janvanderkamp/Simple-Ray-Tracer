#pragma once

#include "surface.h"

class SurfaceGroup : public Surface {
public:
	SurfaceGroup() {}
	SurfaceGroup(Surface **l, int n) { list = l; length = n; }

	virtual bool hit(const Ray& r, float tMin, float tMax, hit_record&rec) const;

public:

	Surface **list;
	int length;
};

bool SurfaceGroup::hit(const Ray& r, float tMin, float tMax, hit_record&rec) const
{
	hit_record temp_r;
	bool hitAny = false;
	double closestSoFar = tMax;

	for (int i = 0; i < length; i++)
	{
		if (list[i]->hit(r, tMin, closestSoFar, temp_r)) {
			hitAny = true;
			closestSoFar = temp_r.t;
			rec = temp_r;
		}
	}

	return hitAny;
}
