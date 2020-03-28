#include <vector>
#include "SDL.h"
#include "tgaimage.h"
#include "ray.h"
#include "surface.h"
#include "surface_group.h"
#include "sphere.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>

// SDL
SDL_Window* window;
SDL_Renderer* renderer;
int done = 1;
float lastTime;
std::string label("aa");

struct Config
{
	int nx, ny, ns;
	Vector3 lowerLeft;
	Vector3 horizontal;
	Vector3 vertical;
	Vector3 origin;
};

float rand_n()
{
	return (float(rand()) / float(RAND_MAX));
}

Vector3 randomInUnitSphere()
{
	Vector3 p;
	do
	{
		p = 2.f * Vector3(rand_n(), rand_n(), rand_n()) - Vector3(1.f);
	} while (p.magnitudeSquared() >= 1.0);

	return p;
}

Vector3 color(const Ray& r, const Surface* world) {

	hit_record rec;
	if (world->hit(r, 0.0, std::numeric_limits < float >::max(), rec))
	{
		const Vector3& n = rec.normal.normalized();
		return 0.5 * Vector3(n.x + 1, n.y + 1, n.z + 1);
	}
	else
	{
		Vector3 unitDir(r.direction());
		unitDir.normalized();
		float t = 0.5f * (unitDir.y + 1.f);
		return (1.f - t) * Vector3(1.0, 1.0, 1.0) + t * Vector3(0.5f, 0.7f, 1.f);
	}
}

void RenderWorld(const Surface& world, const Config& c, TGAImage& image)
{
	TGAColor col;
	for (int j = c.ny - 1; j >= 0; j--)
	{
		for (int i = 0; i < c.nx; i++)
		{
			Vector3 cV(0.f);
			for (int s = 0; s < c.ns; s++)
			{
				float u = (float(i) + rand_n()) / float(c.nx);
				float v = (float(j) + rand_n()) / float(c.ny);

				Ray r(c.origin, c.lowerLeft + u * c.horizontal + v * c.vertical);
				cV += color(r, &world);
			}
			cV /= c.ns;

			int ir = int(255.99f * cV.x);
			int ig = int(255.99f * cV.y);
			int ib = int(255.99f * cV.z);

			col.set(ir, ig, ib);

			//if (j == 50 && i >= 75 && i <= 85)
			//{
			//	printf("i: %d, col: (%d, %d, %d)\n", i, ir, ig, ib);
			//}

			image.set(i, j, col);
		}
	}

	//image.flip_vertically();
}


void
renderLoop(const Surface& world, const Config& config, TGAImage* image, SDL_Texture* framebuffer, bool renderEachFrame = true)
{
	//if (renderEachFrame)
	//{
	//	RenderScene(scene, *image);
	//	float nowSeconds = scene.utils.secondsSinceRun();

	//	cout << "FPS: " << 1.f / (nowSeconds - lastTime) << endl;
	//	lastTime = nowSeconds;
	//}

	//printf(".");

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

			float u = float(x) / float(config.nx);
			float v = float(y) / float(config.ny);
			Ray r(config.origin, config.lowerLeft + u * config.horizontal + v * config.vertical);
			Vector3 cV = color(r, &world);

			printf("colour: (%f, %f, %f)\n", cV.x, cV.y, cV.z, cV);

			//if ((TraceRayRec(scene, testRay, result, 1)))
			//{
			//	//float intensity = LightingForRaycast(scene, result.interectionPoint, result.interectionNormal, -testRay.direction, result.sphere->specularExp);

			//	TGAColor col = result.intersectionColor;
			//	printf("colour: (%d, %d, %d, %d)   pH,pV: (%f, %f)\n", col.bgra[0], col.bgra[1], col.bgra[2], col.bgra[3]);
			//}
		}
		else if (e.type == SDL_KEYUP)
		{
			switch (e.key.keysym.sym)
			{
				//case SDLK_UP:
				//	++ENABLED_FEATURES;
				//	break;
				//case SDLK_DOWN:
				//	--ENABLED_FEATURES;
				//	break;
			case SDLK_ESCAPE:
				done = 1;
				return;
			default:
				break;
			}

			// render frame again if needed
			RenderWorld(world, config, *image);
		}
	}

	RenderWorld(world, config, *image);


	// Rendering code goes here
	SDL_UpdateTexture(framebuffer, NULL, image->buffer(), image->get_width() * image->get_bytespp());

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, framebuffer, NULL, NULL);
	SDL_RenderPresent(renderer);


	/* Got everything on rendering surface,
		now Update the drawing image on window screen */
	SDL_UpdateWindowSurface(window);
}



int main(int argc, char* argv[]) {

	int nx = 1000;
	int ny = 500;
	int ns = 100;
	srand(time(NULL));

	TGAImage image(nx, ny, TGAImage::RGBA);

	Vector3 lowerLeft(-2.f, -1.f, -1.f);
	Vector3 horizontal(4.f, 0.f, 0.f);
	Vector3 vertical(0.f, 2.f, 0.f);
	Vector3 origin(0.f, 0.f, 0.f);

	Config config = { nx, ny, ns, lowerLeft, horizontal, vertical, origin };

	Ray r = Ray(Vector3::zero(), Vector3::zero());

	Surface* list[2];
	list[0] = new Sphere(Vector3(0, 0, -1), 0.5);
	list[1] = new Sphere(Vector3(0, -100.5, -1), 100);
	Surface* world = new SurfaceGroup(list, 2);

	// ==================================
	// Setup SDL
	SDL_Surface* surface;

	/* Enable standard application logging */
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init fail : %s\n", SDL_GetError());
		return 1;
	}

	window = SDL_CreateWindow("Ray Tracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, config.nx, config.ny, 0);
	if (!window) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window creation fail : %s\n", SDL_GetError());
		return 1;
	}
	surface = SDL_GetWindowSurface(window);
	renderer = SDL_CreateSoftwareRenderer(surface);
	if (!renderer) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Render creation for surface fail : %s\n", SDL_GetError());
		return 1;
	}
	SDL_Texture* framebuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, config.nx, config.ny);

	do {
		renderLoop(*world, config, &image, framebuffer, true);
	} while (!done);

	// ==================================

	image.flip_vertically();
	image.write_tga_file(("../results/scene-" + label + ".tga").c_str());

	return 0;
}