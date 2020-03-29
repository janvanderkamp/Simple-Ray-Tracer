#pragma once

#include "surface.h"
#include <math.h>

class Sphere : public Surface {
public:
	Sphere() {}
	Sphere(Vector3 cen, float r, Material * mat) : center(cen), radius(r), material(mat) {};

	virtual bool hit(const Ray& r, float tMin, float tMax, hit_record&rec) const;

	virtual ~Sphere()
	{
		if (material != NULL)
		{
			delete(material);
		}
	}

public:

	void setHit(const Ray& r, float tVal, hit_record& h) const
	{
		h.t = tVal;
		h.p = r.pointAtParameter(h.t);
		h.normal = ((h.p - center) / radius);
		h.mat = material;
	}

	Vector3 center;
	float radius;
	Material * material;
};

bool Sphere::hit(const Ray& r, float tMin, float tMax, hit_record&rec) const
{
	Vector3 oc = r.origin() - center;
	float a = r.direction().dot(r.direction());
	float b = 2.f * oc.dot(r.direction());
	float c = oc.dot(oc) - radius * radius;
	float discriminant = b * b - 4 * a*c;

	if (discriminant > 0)
	{
		float t1 = (-b - sqrt(discriminant)) / (2.f * a);
		float t2 = (-b + sqrt(discriminant)) / (2.f * a);
		float t = fminf(t1, t2);

		if (t < tMax && t > tMin) 
		{
			setHit(r, t, rec);
			return true;
		}
		//if (temp < tMax && temp > tMin)
		//{
		//	setHit(r, temp, rec);
		//	return true;
		//}
	}

	return false;
}