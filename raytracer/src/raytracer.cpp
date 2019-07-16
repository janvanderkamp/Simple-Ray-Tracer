// raytracer.cpp : Defines the entry point for the application.
//

#include "raytracer.h"
#include "tgaimage.h"
#include <algorithm>
#include <vector>

using namespace std;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

// viewport parameters
struct Point3
{
	float x;
	float y;
	float z;

	Point3 operator + (const Point3& rhs) const
	{
		return { x + rhs.x, y + rhs.y, z + rhs.z };
	}
	Point3 operator - (const Point3& rhs) const
	{
		return { x - rhs.x, y - rhs.y, z - rhs.z };
	}
	Point3 operator * (const float rhs) const
	{
		return { x * rhs, y * rhs, z * rhs };
	}
};

struct Point2
{
	int x;
	int y;
};

const Point2 ASPECT_RATIO = { 16.f, 16.f};
const unsigned int CANVAS_WIDTH = 960;
const unsigned int CANVAS_HEIGHT = (CANVAS_WIDTH / ASPECT_RATIO.x) * ASPECT_RATIO.y;
const unsigned int VIEWPORT_WIDTH = 1;
const unsigned int VIEWPORT_HEIGHT = 1;
const unsigned int VIEWPORT_DEPTH = 1;


struct Sphere
{
	Point3 centre;
	float radius;
	TGAColor colour;
};

struct Ray
{
	Point3 origin;
	Point3 direction;
};

struct Light
{
	Point3 posOrDir;
	float intensity;
};

struct IntersectionResult
{
	const Sphere * sphere;
	std::pair<Point3, Point3> interectionPoints;
};

struct Scene
{
	vector<Sphere> spheres;
	vector<Light> directionalLights;
	float ambient = 0.f;
};

float DistanceSquared(const Point3& lhs, const Point3& rhs)
{
	return powf(lhs.x - rhs.x, 2.f) + powf(lhs.y - rhs.y, 2.f) + powf(lhs.z - rhs.z, 2.f);
}

float Magnitude(const Point3& vec)
{
	return sqrtf(powf(vec.x, 2.f) + powf(vec.y, 2.f) + powf(vec.z, 2.f));
}

Point3 Normalize(const Point3& vec)
{
	float magnitude = Magnitude(vec);
	return { vec.x / magnitude, vec.y / magnitude, vec.z / magnitude };
}

float Dot(const Point3& lhs, const Point3& rhs)
{
	return (lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z);
}

bool RaySphereIntersection(const Ray& ray, const Sphere& sphere, std::pair<float, float>& resultOut)
{
	Point3 OC = (ray.origin - sphere.centre);
	Point3 D = (ray.direction);

	float k1 = Dot(D, D);
	float k2 = 2.f * Dot(OC, D);
	float k3 = Dot(OC, OC) - powf(sphere.radius, 2.f);

	float discriminant = (powf(k2, 2.f) - (4 * k1 * k3));
	if (discriminant < 0)
	{
		return false;
	}

	float discriminantRoot = sqrtf(discriminant);
	float divisor = 2.f * k1;
	resultOut.first  = (-k2 + discriminantRoot) / divisor;
	resultOut.second = (-k2 - discriminantRoot) / divisor;

	return true;
}

const Point3 CanvasToViewport(unsigned int x, unsigned int y)
{
	Point3 viewport = { 
		x * ((float)VIEWPORT_WIDTH / (float)CANVAS_WIDTH),
		y * ((float)VIEWPORT_HEIGHT / (float)CANVAS_HEIGHT),
		VIEWPORT_DEPTH
	};

	return viewport;
}

bool TraceRay(const Scene& scene, const Point2& canvasPosition, Ray& shootRay, IntersectionResult& result)
{
	Point3 vpPos = CanvasToViewport(canvasPosition.x, canvasPosition.y);
	shootRay.direction = Normalize(vpPos - shootRay.origin);

	float t = numeric_limits<float>::max();
	const Sphere * firstSphere = nullptr;
	pair<float, float> solution;

	
	for (auto iter = scene.spheres.begin(); iter != scene.spheres.end(); ++iter)
	{
		if (RaySphereIntersection(shootRay, *iter, solution))
		{
			float minT = min(solution.first, solution.second);
			if (minT < t)
			{
				t = minT;
				firstSphere = &(*iter);
			}
		}
	}

	result.sphere = firstSphere;
	result.interectionPoints =
		make_pair(shootRay.origin + shootRay.direction * solution.first,
				  shootRay.origin + shootRay.direction * solution.second);

	return firstSphere != nullptr;
}

float ComputeLight(const Scene& scene, const IntersectionResult& result)
{
	float startingLight = scene.ambient;
	Point3 sphereNormal = Normalize(result.interectionPoints.first - result.sphere->centre);
	for (auto iter = scene.directionalLights.begin(); 
		iter != scene.directionalLights.end(); ++iter)
	{
		float intensity = Dot((*iter).posOrDir, sphereNormal);
		startingLight += max(0.f, intensity);
	}

	return startingLight;
}

// for SDL: see https://gamedev.stackexchange.com/a/157608
/*int main(int argc, char** argv) {

	TGAImage image(CANVAS_WIDTH, CANVAS_HEIGHT, TGAImage::RGB);

	float thresholdSqrd = powf(VIEWPORT_HEIGHT / 4.f, 2.f);

	Point3 zeroVec = { 0.f, 0.f, 0.f };
	Point3 viewportOrigin = { VIEWPORT_WIDTH / 2.f, VIEWPORT_HEIGHT / 2.f, VIEWPORT_DEPTH };
	Point3 viewportX = { 1.f, 0.f, 0.f };

	// test sphere
 	Ray testRay{ { VIEWPORT_WIDTH / 2.f, VIEWPORT_HEIGHT / 2.f, 0.f }, zeroVec };
	std::pair<float, float> intersectResult;

	Scene scene;
	scene.spheres = { 
		{ { 1,  0, 3 }, 1, red },
		{ { 3,  1,  4 }, 1, green},
		{ { -1, 1,  4 }, 1, blue }
	};
	scene.directionalLights = {
		{ Normalize({ 1.f, 1.f, -1.f }), 3.75f }
	};
	scene.ambient = 0.05f;

	TGAColor placeholder = white;
	IntersectionResult result;
	Point2 halfCanvas = { CANVAS_WIDTH / 2.f, CANVAS_HEIGHT / 2.f };
	for (auto x = 0; x < CANVAS_WIDTH; x++)
	{
		for (auto y = 0; y < CANVAS_HEIGHT; y++)
		{
			if ((TraceRay(scene, { x, y }, testRay, result)))
			{
				float intensity = ComputeLight(scene, result);				
				image.set(x, y, result.sphere->colour * intensity);
			}
			else
			{
				image.set(x, y, white);
			}
		}
	}

	image.flip_vertically(); // have the origin at the left bottom corner of the image
	image.write_tga_file("../results/output.tga");
	return 0;
}
//*/