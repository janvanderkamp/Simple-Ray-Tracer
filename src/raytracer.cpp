// raytracer.cpp : Defines the entry point for the application.
//

#include "raytracer.h"
#include "tgaimage.h"

using namespace std;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

// viewport parameters
const unsigned int CANVAS_WIDTH = 800;
const unsigned int CANVAS_HEIGHT = 450;
const unsigned int VIEWPORT_WIDTH = 1;
const unsigned int VIEWPORT_HEIGHT = 1;
const unsigned int VIEWPORT_DEPTH = 1;

struct Point3
{
	float x;
	float y;
	float z;
};

struct Point2
{
	float x;
	float y;
};

const Point3 CanvasToViewport(unsigned int x, unsigned int y)
{
	Point3 viewport = { 
		x * ((float)VIEWPORT_WIDTH / (float)CANVAS_WIDTH),
		y * ((float)VIEWPORT_HEIGHT / (float)CANVAS_HEIGHT),
		VIEWPORT_DEPTH
	};
	return viewport;
}

float DistanceSquared(const Point3& lhs, const Point3& rhs)
{
	return powf(lhs.x - rhs.x, 2.f) + powf(lhs.y - rhs.y, 2.f) + powf(lhs.z - rhs.z, 2.f);
}


int main(int argc, char** argv) {

	TGAImage image(CANVAS_WIDTH, CANVAS_HEIGHT, TGAImage::RGB);

	float thresholdSqrd = powf(VIEWPORT_HEIGHT / 4.f, 2.f);
	Point3 viewportOrigin = { VIEWPORT_WIDTH / 2.f, VIEWPORT_HEIGHT / 2.f, VIEWPORT_DEPTH };
	printf("thresholdSqrd: %f\n", thresholdSqrd);
	for (auto x = 0; x < CANVAS_WIDTH; x++)
	{
		for (auto y = 0; y < CANVAS_HEIGHT; y++)
		{
			Point3 vpPos = CanvasToViewport(x, y);
			float distanceSquared = DistanceSquared(vpPos, viewportOrigin);

			if (x == 350 && y == 290)
			{
				printf("(%d, %d),\t(%f, %f, %f),\t%f\n", x, y, vpPos.x, vpPos.y, vpPos.z, distanceSquared);
			}
			TGAColor fill = distanceSquared <= thresholdSqrd ?
							red : green;
			image.set(x, y, fill);
		}
	}

	image.flip_vertically(); // have the origin at the left bottom corner of the image
	image.write_tga_file("../../results/output.tga");
	return 0;
}
