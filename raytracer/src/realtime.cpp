#include "realtime.h"

//int RealTimeUtils::RunRealTimeScene()
//{
//	return 1;
//}

	//int RealTimeUtils::RunRealTimeScene()
	//{
	//	TGAImage image(CANVAS_WIDTH, CANVAS_HEIGHT, TGAImage::RGBA);
	//	float thresholdSqrd = powf(VIEWPORT_HEIGHT / 4.f, 2.f);

	//	Light light(Vector3());
	//	cout << "size: " << sizeof(Light) << endl;

	//	// setup scene
	//	Point2 halfCanvas = { CANVAS_WIDTH / 2.f, CANVAS_HEIGHT / 2.f };
	//	Vector3 viewportAdjust(VIEWPORT_WIDTH * .5f, VIEWPORT_HEIGHT * .5f, 0.f);

	//	//TextureDetails checkerBoard = {
	//	//	{ Colors.white, Colors.black },
	//	//};

	//	Scene scene;
	//	scene.spheres = {
	//		Sphere(Vector3({ 0, -1,  3 }) + viewportAdjust, 1, 500.f, 0.2f, Colors::red),
	//		Sphere(Vector3({ 2,  0,  4 }) + viewportAdjust, 1, 500.f, 0.3f, Colors::blue),
	//		Sphere(Vector3({ -2, 0,  4 }) + viewportAdjust, 1, 10.f, 0.4f, Colors::green),
	//		//Sphere(Vector3({ .5, 0,  2.5 }) + viewportAdjust, .25f, 10.f, Colors::magenta),
	//		//Sphere(Vector3({ 0, 0.1,  3 }) + viewportAdjust, 0.25f, 500.f, 0.8f, Colors::white),
	//		Sphere(Vector3({ 0, -1001,  0 }) + viewportAdjust, 1000, 1000.f, 0.25f, Colors::yellow)
	//	};
	//	scene.reflectionBounces = 3;

	//	Light ambient;
	//	ambient.intensity = 0.2f;
	//	//Light point(Vector3(2, 1, 0), PointLight, .6f);
	//	Light point(SUN);
	//	Light directional(Vector3(1, 4, 4), DirectionLight, .2f);

	//	scene.lights = {
	//		point,
	//		ambient,
	//		directional
	//	};

	//	RenderScene(scene, image);

	//	// Write to disk also
	//	//image.flip_vertically(); // have the origin at the left bottom corner of the image

	//	SDL_Surface *surface;

	//	/* Enable standard application logging */
	//	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

	//	/* Initialize SDL */
	//	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
	//		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init fail : %s\n", SDL_GetError());
	//		return 1;
	//	}

	//	window = SDL_CreateWindow("Ray Tracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CANVAS_WIDTH, CANVAS_HEIGHT, 0);
	//	if (!window) {
	//		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window creation fail : %s\n", SDL_GetError());
	//		return 1;
	//	}
	//	surface = SDL_GetWindowSurface(window);
	//	renderer = SDL_CreateSoftwareRenderer(surface);
	//	if (!renderer) {
	//		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Render creation for surface fail : %s\n", SDL_GetError());
	//		return 1;
	//	}

	//	SDL_Texture* framebuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, CANVAS_WIDTH, CANVAS_HEIGHT);
	//	loop(scene, &image, framebuffer, true);

	//	bool renderEachFrame = true;
	//	if (!renderEachFrame)
	//	{
	//		image.write_tga_file("../results/output.tga");
	//	}

	//	while (!done) {

	//		if (renderEachFrame)
	//		{
	//			float angle = -scene.utils.secondsSinceRun() * 1.0f;
	//			//float angle = 0.f;

	//			// rotate point
	//			scene.lights[0].position.x = cosf(angle) * SUN.position.x - sinf(angle) * SUN.position.z;
	//			scene.lights[0].position.z = sinf(angle) * SUN.position.x + cosf(angle) * SUN.position.z;
	//			scene.lights[0].position.normalize();

	//			//printf("SUN: (%f, %f, %f)\n", scene.lights[0]);
	//		}

	//		loop(scene, &image, framebuffer, renderEachFrame);
	//	}
	//}
