#include "SDL.h"
#include "src/tgaimage.h"

SDL_Window *window;
SDL_Renderer *renderer;
int done;

void
DrawChessBoard(SDL_Renderer * renderer)
{
    int row = 0,column = 0,x = 0;
    SDL_Rect rect, darea;

    /* Get the Size of drawing surface */
    SDL_RenderGetViewport(renderer, &darea);

    for ( ; row < 8; row++) {
        column = row%2;
        x = column;
        for ( ; column < 4+(row%2); column++) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);

            rect.w = darea.w/8;
            rect.h = darea.h/8;
            rect.x = x * rect.w;
            rect.y = row * rect.h;
            x = x + 2;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void
//loop(const TGAImage& image)
loop(uint32_t * buffer, SDL_Texture* framebuffer, int width)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            done = 1;
            return;
        }

        if ((e.type == SDL_KEYDOWN) && (e.key.keysym.sym == SDLK_ESCAPE)) {
            done = 1;
            return;
        }
    }

    //DrawChessBoard(renderer);

	// Rendering code goes here
	SDL_UpdateTexture(framebuffer, NULL, buffer, width * sizeof(uint32_t));

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, framebuffer, NULL, NULL);
	SDL_RenderPresent(renderer);


    /* Got everything on rendering surface,
       now Update the drawing image on window screen */
    SDL_UpdateWindowSurface(window);
}

uint32_t ToUint(const TGAColor& c)
{
	// RGBA format
	return (uint32_t)(((c.bgra[3] << 24) | (c.bgra[2] << 16) | (c.bgra[1] << 8) | c.bgra[0]) & 0xffffffffL);
}

bool SetColor(int x, int y, int width, int bytespp, uint32_t * buffer, const TGAColor& c) {

	memcpy(buffer + (x + y * width) * bytespp, c.bgra, bytespp);
	return true;
}

int
main(int argc, char *argv[])
{
    SDL_Surface *surface;

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init fail : %s\n", SDL_GetError());
        return 1;
    }

    /* Create window and renderer for given surface */
	int width = 640, height = 480;
	uint32_t * buffer = new uint32_t[width * height];
	unsigned long nbytes = width * height * 4;
	memset(buffer, 122, nbytes);

	//TGAImage image(width, height, TGAImage::RGBA);
	//for (auto x = 0; x < width; x++)
	//{
	//	for (auto y = 0; y < height; y++)
	//	{
	//		image.set(x, y, Colors::yellow);
	//	}
	//}

    window = SDL_CreateWindow("Chess Board", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
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

    /* Clear the rendering surface with the specified color */
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    /* Draw the Image on rendering surface */
    done = 0;

	SDL_Texture* framebuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA8888, SDL_TEXTUREACCESS_STREAMING, width, height);


    while (!done) {
        loop(buffer, framebuffer, width);
    }

	delete[] buffer;

    SDL_Quit();
    return 0;
}