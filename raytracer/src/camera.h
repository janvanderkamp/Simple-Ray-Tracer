#pragma once

#include "ray.h"

class camera {
public:
	camera() {
		lowerLeft = Vector3(-2.0, -1.0, -1.0);
		horizontal = Vector3(4.0, 0, 0);
		vertical = Vector3(0, 2.0, 0);
		origin = Vector3::zero();
	}

	Ray getRay(float u, float v)
	{
		return Ray(origin, lowerLeft + u * horizontal + v * vertical - origin);
	}

	Vector3 origin;
	Vector3 lowerLeft;
	Vector3 horizontal;
	Vector3 vertical;
};