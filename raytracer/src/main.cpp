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


struct Point2
{
	int x;
	int y;
};

const Point2 ASPECT_RATIO = { 16.f, 16.f};
const unsigned int CANVAS_WIDTH = 540;
const unsigned int CANVAS_HEIGHT = (CANVAS_WIDTH / ASPECT_RATIO.x) * ASPECT_RATIO.y;
const unsigned int VIEWPORT_WIDTH = 1;
const unsigned int VIEWPORT_HEIGHT = 1;
const unsigned int VIEWPORT_DEPTH = 1;
//Utils utils;

enum LightType
{
	PointLight,
	DirectionLight,
	SpotLight,
	Ambient
};

struct Sphere
{
	Vector3 centre;
	float radius;
	float specularExp;
	TGAColor colour;
};

struct Ray
{
	Vector3 origin;
	Vector3 direction;
};

struct Light
{
	Light() :
		posOrDir(Vector3()),
		intensity(0.f),
		color(Colors::white),
		spotlightAngle(0.f),
		range(0.f),
		type(LightType::Ambient) {
	};
	Light(const Vector3& posOrDir,	float intensity=1.f, const TGAColor& color=Colors::white, float spotlightAngle=360.f, float range=1.f, LightType type=LightType::DirectionLight) :
		posOrDir(posOrDir),
		intensity(intensity),
		color(color),
		spotlightAngle(spotlightAngle),
		range(range),
		type(type) {
	};
	Vector3 posOrDir;
	float intensity;
	TGAColor color;
	float spotlightAngle;
	float range;
	LightType type;
};

struct IntersectionResult
{
	const Sphere * sphere;
	std::pair<Vector3, Vector3> interectionPoints;
};

struct Scene
{
	vector<Sphere> spheres;
	vector<Light> lights;
	float ambient = 0.f;
};

float DistanceSquared(const Vector3& lhs, const Vector3& rhs)
{
	return powf(lhs.x - rhs.x, 2.f) + powf(lhs.y - rhs.y, 2.f) + powf(lhs.z - rhs.z, 2.f);
}

float Magnitude(const Vector3& vec)
{
	return sqrtf(powf(vec.x, 2.f) + powf(vec.y, 2.f) + powf(vec.z, 2.f));
}

bool RaySphereIntersection(const Ray& ray, const Sphere& sphere, std::pair<float, float>& resultOut)
{
	Vector3 OC = (ray.origin - sphere.centre);
	Vector3 D = (ray.direction);

	float k1 = D.dot(D);
	float k2 = 2.f * (OC.dot(D));
	float k3 = (OC.dot(OC)) - powf(sphere.radius, 2.f);

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

const Vector3 CanvasToViewport(unsigned int x, unsigned int y)
{
	Vector3 viewport(x * ((float)VIEWPORT_WIDTH / (float)CANVAS_WIDTH),
		y * ((float)VIEWPORT_HEIGHT / (float)CANVAS_HEIGHT),
		VIEWPORT_DEPTH);
	
	return viewport;
}

bool TraceRay(const Scene& scene, const Point2& canvasPosition, Ray& shootRay, IntersectionResult& result)
{
	Vector3 vpPos = CanvasToViewport(canvasPosition.x, canvasPosition.y);
	shootRay.direction = (vpPos - shootRay.origin).normalized();

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

float GetDiffuse(const Vector3& normalN, const Vector3& lightN )
{
	float NDotL = max(lightN.dot(normalN), 0.f);
	return NDotL;
}

float GetSpecular(const Vector3& normalN, const Vector3& lightN, const Vector3& viewN, float specularPower)
{
	Vector3 reflectingVecN = ((normalN * 2.f) * (normalN.dot(lightN)) - lightN).normalized();
	float RDotV = max(reflectingVecN.dot(viewN), 0.f);

	return powf(RDotV, specularPower);
}

float GetLighting(const Vector3& normalN, const Vector3& viewVecN, const Vector3& lightDirN, float specularExp)
{
	float diffuse = GetDiffuse(normalN, lightDirN);

	Vector3 reflectingVecN = ((normalN * 2.f) * (normalN.dot(lightDirN)) - lightDirN).normalized();
	float specularIntensity = GetSpecular(normalN, lightDirN, viewVecN, specularExp);

	return (diffuse + specularIntensity);
}

float LightingForRaycast(const Scene& scene, const IntersectionResult& result, const Vector3& viewVecN)
{
	float startingLight = scene.ambient;
	Vector3 sphereNormal = (result.interectionPoints.first - result.sphere->centre).normalized();

	for (auto iter = scene.lights.begin(); 
		iter != scene.lights.end(); ++iter)
	{
		switch (iter->type)
		{
		case LightType::Ambient:
			break;
		case LightType::DirectionLight:
			break;
		case LightType::PointLight:
			break;

				
		}

		const Vector3& lightDir = -(*iter).posOrDir;
		float intensity = GetLighting(sphereNormal, viewVecN, lightDir, result.sphere->specularExp);

		startingLight += intensity;
	}

	return startingLight;
}

void RenderScene(const Scene& scene, TGAImage& image)
{
	// Useful variables
	Vector3 zeroVec = { 0.f, 0.f, 0.f };
	Vector3 viewportOrigin( VIEWPORT_WIDTH / 2.f, VIEWPORT_HEIGHT / 2.f, VIEWPORT_DEPTH );
	Vector3 viewportX = { 1.f, 0.f, 0.f };

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
				float intensity = LightingForRaycast(scene, result, -testRay.direction.normalized());
				image.set(x, y, result.sphere->colour * intensity);
			}
			else
			{
				image.set(x, y, Colors::white);
			}
		}
	}

	image.flip_vertically();
}

// SDL
SDL_Window *window;
SDL_Renderer *renderer;
int done;
float lastTime;

// Extras
const Light SUN = { Vector3(-.25f, -0.f, -1.f).normalized(), 6.75f };


void
loop(const Scene& scene, const Utils& utils, TGAImage * image, SDL_Texture* framebuffer, bool render=true)
{
	if (render)
	{
		RenderScene(scene, *image);
		float nowSeconds = utils.secondsSinceRun();

		cout << "FPS: " << 1.f / (nowSeconds - lastTime) << endl;
		lastTime = nowSeconds;
	}

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
			x = 168;
			y = 359;

			if ((TraceRay(scene, { x, y }, testRay, result)))
			{
				float intensity = LightingForRaycast(scene, result, -testRay.direction);
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

	Light light(Vector3());
	cout << "size: " << sizeof(Light) << endl;

	Scene scene;
	scene.spheres = { 
		{ { 0,  0, 3 }, 1, 500.f, Colors::red },
		{ { 2,  1,  4 }, 1, 500.f, Colors::blue},
		{ { -2, 1,  4 }, 1, 10.f, Colors::green }
	};
	
	Light ambient;
	ambient.intensity = 0.2f;
	Light point(Vector3(2, 1, 0), .6f);
	Light directional(Vector3(1, 4, 4), .2f);

	scene.lights = {
		ambient,
		point,
		directional
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
	loop(scene, utils, &image, framebuffer, true);

	bool render = false;
	while (!done) {

		if (render)
		{
			float angle = -utils.secondsSinceRun() * 1.0f;
			//float angle = 0.f;

			// rotate sun
			scene.lights[0].posOrDir.x = cosf(angle) * SUN.posOrDir.x - sinf(angle) * SUN.posOrDir.z;
			scene.lights[0].posOrDir.z = sinf(angle) * SUN.posOrDir.x + cosf(angle) * SUN.posOrDir.z;

			printf("SUN: (%f, %f, %f)\n", scene.lights[0]);
		}

		loop(scene, utils, &image, framebuffer, render);
	}

	return 0;
}
//*/