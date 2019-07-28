// raytracer.cpp : Defines the entry point for the application.
//
#include <algorithm>
#include <vector>
#include "SDL.h"
#include "raytracer.h"
#include "tgaimage.h"
#include "vector3.h"
#include "utils.h"

using namespace std;


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
//Utils utils;


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
				// just break here, these spheres aren't transparent
				t = minT;
				firstSphere = &(*iter);
				break;
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

void RenderScene(const Scene& scene, TGAImage& image)
{
	// Useful variables
	Point3 zeroVec = { 0.f, 0.f, 0.f };
	Point3 viewportOrigin = { VIEWPORT_WIDTH / 2.f, VIEWPORT_HEIGHT / 2.f, VIEWPORT_DEPTH };
	Point3 viewportX = { 1.f, 0.f, 0.f };

	Vector3 zero(VIEWPORT_WIDTH / 2.f, VIEWPORT_HEIGHT / 2.f, VIEWPORT_DEPTH);
	Vector3 one(1.f);
	Vector3 defaultA;
	defaultA.zero();
	one = zero = Vector3(1, 2, 3);

	Ray testRay = { { VIEWPORT_WIDTH / 2.f, VIEWPORT_HEIGHT / 2.f, 0.f }, zeroVec };
	std::pair<float, float> intersectResult;

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
				image.set(x, y, Colors::white);
			}
		}
	}
}

// SDL
SDL_Window *window;
SDL_Renderer *renderer;
int done;
unsigned __int64 lastTime;

// Extras
const Light SUN = { Normalize({ 1.f, 1.f, -1.f }), 3.75f };


void
loop(const Scene& scene, TGAImage * image, SDL_Texture* framebuffer)
{
	float nowSeconds = Utils().secondsSinceRun();

	// cout << "FPS: " << 1000.f / (float)millis << endl;

	// debugging
	IntersectionResult result;
	Ray testRay = { { VIEWPORT_WIDTH / 2.f, VIEWPORT_HEIGHT / 2.f, 0.f }, {0.f, 0.f, 0.f} };

	// poll events
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT) {
			done = 1;
			return;
		}

		if (e.type == SDL_MOUSEBUTTONUP)
		{
			//Get mouse position
			int x, y;
			SDL_GetMouseState(&x, &y);

			if ((TraceRay(scene, { x, y }, testRay, result)))
			{
				float intensity = ComputeLight(scene, result);
				TGAColor col = result.sphere->colour * intensity;
				printf("intensity: %f, colour: (%d, %d, %d, %d)\n", intensity, col.bgra[0], col.bgra[1], col.bgra[2], col.bgra[3]);
			}
		}

		if ((e.type == SDL_KEYDOWN) && (e.key.keysym.sym == SDLK_ESCAPE)) {
			done = 1;
			return;
		}
	}

	// Rendering code goes here
	SDL_UpdateTexture(framebuffer, NULL, image->buffer(), image->get_width() * image->get_bytespp());

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, framebuffer, NULL, NULL);
	SDL_RenderPresent(renderer);


	/* Got everything on rendering surface,
	   now Update the drawing image on window screen */
	SDL_UpdateWindowSurface(window);
}

//*
int main(int argc, char *argv[]) {

	TGAImage image(CANVAS_WIDTH, CANVAS_HEIGHT, TGAImage::RGBA);
	float thresholdSqrd = powf(VIEWPORT_HEIGHT / 4.f, 2.f);

	Scene scene;
	scene.spheres = { 
		{ { 1,  0, 3 }, 1, Colors::red },
		{ { 3,  1,  4 }, 1, Colors::green},
		{ { -1, 1,  4 }, 1, Colors::blue }
	};
	scene.directionalLights = {
		SUN
	};
	scene.ambient = 0.25f;

	RenderScene(scene, image);

	// Write to disk also
	//image.flip_vertically(); // have the origin at the left bottom corner of the image
	//image.write_tga_file("../results/output.tga");

    SDL_Surface *surface;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init fail : %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Ray Tracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CANVAS_WIDTH, CANVAS_HEIGHT, 0);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window creation fail : %s\n",SDL_GetError());
        return 1;
    }
    surface = SDL_GetWindowSurface(window);
    renderer = SDL_CreateSoftwareRenderer(surface);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Render creation for surface fail : %s\n",SDL_GetError());
        return 1;
    }

	Utils utils;
	SDL_Texture* framebuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, CANVAS_WIDTH, CANVAS_HEIGHT);
	while (!done) {

		float angle = -utils.secondsSinceRun() * .003f;
		//float angle = 0.f;
		//printf("time: %ld\n", utils.epochSeconds());
		printf("time: %f\n", utils.secondsSinceRun());

		// rotate sun
		scene.directionalLights[0].posOrDir.x = cosf(angle) * SUN.posOrDir.x - sinf(angle) * SUN.posOrDir.z;
		scene.directionalLights[0].posOrDir.z = sinf(angle) * SUN.posOrDir.x + cosf(angle) * SUN.posOrDir.z;

		//printf("SUN: (%f, %f, %f)\n", scene.directionalLights[0]);

		loop(scene, &image, framebuffer);
	}

	return 0;
}
//*/