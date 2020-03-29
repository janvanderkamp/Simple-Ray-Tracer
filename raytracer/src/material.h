#pragma once
#include "ray.h"
#include "surface.h"
#include "utils.h"

class Material
{
public:
	virtual bool scatter(const Ray& rayIn, const hit_record& rec, Vector3& attenuation, Ray& scattered) = 0;
};

class Lambertian : public Material
{
public:
	Lambertian(Vector3 albedo) : _albedo(albedo) {}

	virtual bool scatter(const Ray & rayIn, const hit_record & rec, Vector3 & attenuation, Ray & scattered)
	{
		Vector3 target = rec.p + rec.normal + Utils::randomInUnitSphere();
		scattered = Ray(rec.p, target - rec.p);
		attenuation = _albedo;
		return true;
	}

private:
	Vector3 _albedo;
};

class Metal : public Material
{
public:
	Metal(Vector3 albedo) : _albedo(albedo) {}

	virtual bool scatter(const Ray & rayIn, const hit_record & rec, Vector3 & attenuation, Ray & scattered)
	{
		Vector3 reflected = rayIn.direction().normalized().reflect(rec.normal);
		scattered = Ray(rec.p, reflected);
		attenuation = _albedo;
		return true;
	}

private:
	Vector3 _albedo;
};