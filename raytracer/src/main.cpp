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


enum LightType
{
	PointLight,
	DirectionLight,
	SpotLight,
	AmbientLight
};

enum Features
{
	Color,
	Ambient,
	Diffuse,
	Specular,
	Shadows,
	Reflection
};

Features& operator++(Features& f)
{
	return f = (f == Features::Reflection) ? Features::Color : static_cast<Features>(static_cast<int>(f) + 1);
}
Features& operator--(Features& f)
{
	return f = (f == Features::Color) ? Features::Reflection : static_cast<Features>(static_cast<int>(f) - 1);
}

struct Point2
{
	int x;
	int y;
};

const Point2 ASPECT_RATIO = { 16.f, 16.f};
const TGAColor CLEAR_COL = Colors::skyBlue;
const int CANVAS_WIDTH = 720;
const int CANVAS_HEIGHT = (CANVAS_WIDTH / ASPECT_RATIO.x) * ASPECT_RATIO.y;
const int VIEWPORT_WIDTH = 1;
const int VIEWPORT_HEIGHT = 1;
const int VIEWPORT_DEPTH = 1;
const float EPSILON = .0001f;

// TODO: these all in world space for sphere texturing
const Vector3 UP(0.f, 1.f, 0.f);
const Vector3 FORWARD(0.f, 0.f, -1.f);

Features ENABLED_FEATURES = Reflection;
//Utils utils;

class Sphere
{
public:
	Sphere(const Vector3& centre, float radius, float specularExp, float reflective, TGAColor color, bool textureDetails = false) :
		centre(centre),
		radius(radius),
		specularExp(specularExp),
		reflective(reflective),
		color(color),
		textureDetails(textureDetails)
	{}

	Vector3 centre;
	float radius;
	float specularExp;
	float reflective;
	bool textureDetails;

	TGAColor getColorAtPoint(const Vector3& point) const
	{
		if (textureDetails)
		{
			float texScale = radius * 4.f;
			Vector3 cToP = (point - centre).normalized();
			float pH = FORWARD.dot(cToP);
			float pV = UP.dot(cToP);
			if (cToP.y < UP.y)
			{
				pV *= -1.f;
			}

			int uv1 = static_cast<int>((pH * texScale));
			int uv2 = static_cast<int>((pV * texScale));

			bool setCol = uv2 % 2 == 0 ? uv1 % 2 != 0 : uv1 % 2 == 0;

			TGAColor texcol = uv1 % 2 != 0 ? Colors::white : Colors::black ;
			return texcol;
		}

		return color;
	}

private:
	TGAColor color;
};

struct Ray
{
	Vector3 origin;
	Vector3 direction;
};

struct Light
{
	Light() :
		position(),
		directionN(),
		intensity(0.f),
		color(Colors::white),
		spotlightAngle(0.f),
		range(0.f),
		type(LightType::AmbientLight) {
	};
	Light(const Vector3& posOrDir,	LightType type=LightType::DirectionLight, float intensity=1.f, const TGAColor& color=Colors::white, float spotlightAngle=360.f, float range=1.f) :
		position(),
		directionN(),
		intensity(intensity),
		color(color),
		spotlightAngle(spotlightAngle),
		range(range),
		type(type) 
	{
		if (type == LightType::DirectionLight)
		{
			directionN = posOrDir;
			directionN.normalize();
		}
		else if (type == LightType::PointLight)
		{
			position = posOrDir;
		}
	};

	Vector3 position;
	Vector3 directionN;
	float intensity;
	TGAColor color;
	float spotlightAngle;
	float range;
	LightType type;
};

struct IntersectionResult
{
	const Sphere * sphere;
	TGAColor intersectionColor;

	// TODO: store these together
	Vector3 intersectionPoint;
	Vector3 interectionNormal;
};

struct Scene
{
	vector<Sphere> spheres;
	vector<Light> lights;
	int reflectionBounces;
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

const Vector3 CanvasToViewport(int x, int y)
{
	Vector3 viewport(x * ((float)VIEWPORT_WIDTH / (float)CANVAS_WIDTH),
					 y * ((float)VIEWPORT_HEIGHT / (float)CANVAS_HEIGHT),
					 VIEWPORT_DEPTH);
	
	return viewport ;
}

bool DoesIntersectSphere(const Scene& scene, Ray& shootRay, IntersectionResult& result, float minT = 0.f, float maxT = numeric_limits<float>::max(), bool checkAll=true)
{
	const Sphere * firstSphere = nullptr;
	pair<float, float> solution;
	float t = maxT;

	for (auto iter = scene.spheres.begin(); iter != scene.spheres.end(); ++iter)
	{
		if (RaySphereIntersection(shootRay, *iter, solution))
		{
			float first = min(solution.first, solution.second);
			if (first >= minT && first < t)
			{
				// just break here, these spheres aren't transparent
				t = first;
				firstSphere = &(*iter);

				if (!checkAll)
				{
					break;
				}
			}
		}
	}

	result.sphere = firstSphere;
	result.intersectionPoint = shootRay.origin + shootRay.direction * t;

	return firstSphere != nullptr;
}

float GetDiffuse(const Vector3& normalN, const Vector3& lightN )
{
	float NDotL = max(normalN.dot(lightN), 0.f);
	return NDotL;
}

float GetSpecular(const Vector3& normalN, const Vector3& lightN, const Vector3& viewN, float specularPower)
{
	//Vector3 reflectingVecN = ((normalN * 2.f) * (normalN.dot(lightN)) - lightN).normalized();
	Vector3 reflectingVecN = (-lightN).reflect(normalN);

	float RDotV = max(reflectingVecN.dot(viewN), 0.f);

	return powf(RDotV, specularPower);
}

float GetLighting(const Vector3& normalN, const Vector3& viewVecN, const Vector3& lightDirN, float intensity, float specularExp)
{
	float amount = ENABLED_FEATURES >= Diffuse ? 
				   GetDiffuse(normalN, lightDirN) : 
				   0.f ;

	if (ENABLED_FEATURES >= Specular)
	{
		Vector3 reflectingVecN = ((normalN * 2.f) * (normalN.dot(lightDirN)) - lightDirN).normalized();
		float specularIntensity = GetSpecular(normalN, lightDirN, viewVecN, specularExp);
		amount += specularIntensity;
	}

	return amount * intensity;
}

// TODO: anything intersection related together, specular in material
float LightingForRaycast(const Scene& scene, const Vector3& intersectionPoint, const Vector3& intersectionNormalN,  const Vector3& viewVecN, float specular)
{
	float sceneLight = 0.f;
	//const Vector3& intersection = result.interectionPoint;
	//Vector3 sphereNormal = (intersection - result.sphere->centre).normalized();

	for (auto iter = scene.lights.begin(); 
		iter != scene.lights.end(); ++iter)
	{
		float lightContribution = 0.f;

		switch (iter->type)
		{
		case LightType::AmbientLight:
			sceneLight += ENABLED_FEATURES >= Ambient ? iter->intensity : 0.f;
			break;
		case LightType::DirectionLight:
		{
			IntersectionResult shadowIntersection;
			const Vector3& lightDir = iter->directionN;
			Ray shadowRay = { intersectionPoint, lightDir };

			// If nothing blocking us then not in shadow
			if (ENABLED_FEATURES >= Shadows ?
				!DoesIntersectSphere(scene, shadowRay, shadowIntersection, EPSILON, numeric_limits<float>::max(), false) : 
				true)
			{
				lightContribution = GetLighting(intersectionNormalN, viewVecN, lightDir, iter->intensity, specular);
			}

			break;
		}
		case LightType::PointLight:

			IntersectionResult shadowIntersection;
			Vector3 lightDir = (iter->position - intersectionPoint);
			Ray shadowRay = { intersectionPoint, lightDir };

			// If nothing blocking us then not in shadow
			if (ENABLED_FEATURES >= Shadows ?
				!DoesIntersectSphere(scene, shadowRay, shadowIntersection, EPSILON, 1.f, false) :
				true)
			{
				lightContribution = GetLighting(intersectionNormalN, viewVecN, lightDir.normalized(), iter->intensity, specular);
			}
			break;
		}

		sceneLight += lightContribution;
	}

	return sceneLight;
}

bool TraceRay(const Scene& scene, const Point2& canvasPosition, Ray& shootRay, IntersectionResult& result)
{
	Vector3 vpPos = CanvasToViewport(canvasPosition.x, canvasPosition.y);
	shootRay.direction = (vpPos - shootRay.origin).normalized();

	if (DoesIntersectSphere(scene, shootRay, result, 1.f))
	{
		Vector3 sphereNormal = (result.intersectionPoint - result.sphere->centre).normalized();

		float intensity = ENABLED_FEATURES > Color ?
						/*LightingForRaycast(scene, result, -shootRay.direction.normalized()) :*/
						LightingForRaycast(scene, result.intersectionPoint, sphereNormal, -shootRay.direction.normalized(), result.sphere->specularExp) :
						1.f;

		// Now set the intersection colour
		result.intersectionColor = result.sphere->getColorAtPoint(result.intersectionPoint) * intensity;

		return true;
	}

	return false;
}

bool TraceRayRec(const Scene& scene, Ray& shootRay, IntersectionResult& result, int numBouncesLeft = 0, float minT = 1.f)
{
	if (DoesIntersectSphere(scene, shootRay, result, minT))
	{
		Vector3 sphereNormal = (result.intersectionPoint - result.sphere->centre).normalized();

		float intensity =	ENABLED_FEATURES > Color ?
							LightingForRaycast(scene, result.intersectionPoint, sphereNormal, -shootRay.direction.normalized(), result.sphere->specularExp) :
							1.f;


		TGAColor intersectionColourCurr = result.sphere->getColorAtPoint(result.intersectionPoint) * intensity;
		if (numBouncesLeft > 0)
		{
			IntersectionResult reflectResult;
			shootRay.origin = result.intersectionPoint;
			shootRay.direction = shootRay.direction.reflect(sphereNormal);

			TGAColor intersectionColourNext = CLEAR_COL;
			float lerpFactor = result.sphere->reflective;
			if ((lerpFactor > EPSILON) && TraceRayRec(scene, shootRay, reflectResult, numBouncesLeft - 1, EPSILON))
			{
				intersectionColourNext = reflectResult.sphere->getColorAtPoint(result.intersectionPoint) * intensity;
			}

			TGAColor mixed;
			TGAColor::lerp(intersectionColourNext, intersectionColourCurr, lerpFactor, &mixed);
			result.intersectionColor = mixed;
		}
		else
		{
			result.intersectionColor = intersectionColourCurr;
		}

		return true;
	}

	return false;
}

void RenderScene(const Scene& scene, TGAImage& image)
{
	// Useful variables
	Vector3 zeroVec = { 0.f, 0.f, 0.f };

	Ray testRay = { { VIEWPORT_WIDTH / 2.f, VIEWPORT_HEIGHT / 2.f, 0.f }, zeroVec };
	std::pair<float, float> intersectResult;

	IntersectionResult result;
	for (auto x = 0; x < CANVAS_WIDTH; ++x)
	{
		for (auto y = CANVAS_HEIGHT - 1; y >= 0; --y)
		{
			int invY = CANVAS_HEIGHT - y;

			Vector3 vpPos = CanvasToViewport(x, invY);
			//testRay.direction = (vpPos - testRay.origin).normalized();

			//if ((TraceRay(scene, { x, invY }, testRay, result)))
			//{
			//	image.set(x, y, result.intersectionColor);
			//}
			//else
			//{
			//	image.set(x, y, Colors::white);
			//}

			Ray testRay = { { VIEWPORT_WIDTH / 2.f, VIEWPORT_HEIGHT / 2.f, 0.f }, zeroVec };
			testRay.direction = (vpPos - testRay.origin).normalized();

			if ((TraceRayRec(scene, testRay, result, ENABLED_FEATURES >= Reflection ? 2 : 0 )))
			{
				image.set(x, y, result.intersectionColor);
			}
			else
			{
				image.set(x, y, CLEAR_COL);
			}
		}
	}

	//image.flip_vertically();
}

// SDL
SDL_Window *window;
SDL_Renderer *renderer;
int done;
float lastTime;

// Extras
const Light SUN(Vector3(12, 1, 0), PointLight, .6f);

void
loop(const Scene& scene, const Utils& utils, TGAImage * image, SDL_Texture* framebuffer, bool renderEachFrame=true)
{
	if (renderEachFrame)
	{
		RenderScene(scene, *image);
		float nowSeconds = utils.secondsSinceRun();

		cout << "FPS: " << 1.f / (nowSeconds - lastTime) << endl;
		lastTime = nowSeconds;
	}

	// debugging
	IntersectionResult result;

	// poll events
	SDL_Event e;
	bool dirty = false;
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

			// TODO: Ray constructor
			Vector3 vpPos = CanvasToViewport( x, CANVAS_HEIGHT - y );
			Ray testRay = { { VIEWPORT_WIDTH / 2.f, VIEWPORT_HEIGHT / 2.f, 0.f },  (vpPos - testRay.origin).normalized() };

			if ((TraceRayRec(scene, testRay, result, 1)))
			{
				//float intensity = LightingForRaycast(scene, result.interectionPoint, result.interectionNormal, -testRay.direction, result.sphere->specularExp);

				TGAColor col = result.intersectionColor;
				printf("colour: (%d, %d, %d, %d)   pH,pV: (%f, %f)\n", col.bgra[0], col.bgra[1], col.bgra[2], col.bgra[3]);
			}
		}
		else if (e.type == SDL_KEYUP)
		{
			switch (e.key.keysym.sym)
			{
			case SDLK_UP:
				++ENABLED_FEATURES;
				break;
			case SDLK_DOWN:
				--ENABLED_FEATURES;
				break;
			case SDLK_ESCAPE:
				done = 1;
				return;
			default:
				break;
			}

			if (!renderEachFrame)
			{
				dirty = true;
			}

			// render frame again if needed
			if (dirty)
			{
				RenderScene(scene, *image);
			}
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

	// setup scene
	Point2 halfCanvas = { CANVAS_WIDTH / 2.f, CANVAS_HEIGHT / 2.f };
	Vector3 viewportAdjust(VIEWPORT_WIDTH * .5f, VIEWPORT_HEIGHT * .5f, 0.f);

	//TextureDetails checkerBoard = {
	//	{ Colors.white, Colors.black },
	//};


	Scene scene;
	scene.spheres = { 
		//{ Vector3({ 0, -1,  3 }) + viewportAdjust, 1, 500.f, 0.2f, Colors::red },
		//{ Vector3({ 2,  0,  4 }) + viewportAdjust, 1, 500.f, 0.3f, Colors::blue},
		//{ Vector3({ -2, 0,  4 }) + viewportAdjust, 1, 10.f, 0.4f, Colors::green },
		//{ Vector3({ .5, 0,  2.5 }) + viewportAdjust, .25, 10.f, Colors::magenta },
		Sphere(Vector3({ 0, 0.1,  3 }) + viewportAdjust, 0.25f, 500.f, 0.8f, Colors::white),
		Sphere(Vector3({ 0, -1001,  0 }) + viewportAdjust, 1000, 1000.f, 0.25f, Colors::yellow, true)
	};
	scene.reflectionBounces = 3;
	
	Light ambient;
	ambient.intensity = 0.2f;
	//Light point(Vector3(2, 1, 0), PointLight, .6f);
	Light point(SUN);
	Light directional(Vector3(1, 4, 4), DirectionLight, .2f);

	scene.lights = {
		point,
		ambient,
		directional
	};

	RenderScene(scene, image);

	// Write to disk also
	//image.flip_vertically(); // have the origin at the left bottom corner of the image

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

	bool renderEachFrame = false;
	if (!renderEachFrame)
	{
		image.write_tga_file("../results/output.tga");
	}

	while (!done) {

		if (renderEachFrame)
		{
			float angle = -utils.secondsSinceRun() * 1.0f;
			//float angle = 0.f;

			// rotate point
			scene.lights[0].position.x = cosf(angle) * SUN.position.x - sinf(angle) * SUN.position.z;
			scene.lights[0].position.z = sinf(angle) * SUN.position.x + cosf(angle) * SUN.position.z;
			scene.lights[0].position.normalize();

			//printf("SUN: (%f, %f, %f)\n", scene.lights[0]);
		}

		loop(scene, utils, &image, framebuffer, renderEachFrame);
	}

	return 0;
}
//*/