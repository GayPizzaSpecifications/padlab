#include "draw.h"
#include "maths.h"
#include <SDL_render.h>
#include <stdlib.h>


static SDL_Renderer* rend = NULL;

void DrawWindowHints(void) {}

int InitDraw(SDL_Window* window)
{
	const int rendflags = SDL_RENDERER_PRESENTVSYNC;
	rend = SDL_CreateRenderer(window, NULL, rendflags);
	return (rend == NULL) ? -1 : 0;
}

void QuitDraw(void)
{
	SDL_DestroyRenderer(rend);
	rend = NULL;
}


void SetDrawViewport(size size) {}


void SetDrawColour(uint32_t c)
{
	SDL_SetRenderDrawColor(rend,
		(c & 0xFF000000) >> 24,
		(c & 0x00FF0000) >> 16,
		(c & 0x0000FF00) >>  8,
		(c & 0x000000FF));
}

void DrawClear(void)
{
	SDL_RenderClear(rend);
}

void DrawPoint(int x, int y)
{
	SDL_RenderPoint(rend, (float)x, (float)y);
}

void DrawRect(int x, int y, int w, int h)
{
	SDL_FRect dst = {
		.x = (float)x, .y = (float)y,
		.w = (float)w, .h = (float)h };
	SDL_RenderRect(rend, &dst);
}

void DrawLine(int x1, int y1, int x2, int y2)
{
	SDL_RenderLine(rend, (float)x1, (float)y1, (float)x2, (float)y2);
}

void DrawCircleSteps(int x, int y, int r, int steps)
{
	double stepsz = (double)TAU / steps;
	int lastx = r;
	int lasty = 0;
	for (int i = 1; i <= steps; ++i)
	{
		const double mag = (double)r;
		int ofsx = (int)round(cos(stepsz * i) * mag);
		int ofsy = (int)round(sin(stepsz * i) * mag);

		SDL_RenderLine(rend,
			(float)(x + lastx), (float)(y + lasty),
			(float)(x + ofsx),  (float)(y + ofsy));

		lastx = ofsx;
		lasty = ofsy;
	}
}

void DrawArcSteps(int x, int y, int r, int startAng, int endAng, int steps)
{
	const double fstart = (double)startAng * DEG2RAD;
	const double fstepSz = (double)(endAng - startAng) / abs(steps) * DEG2RAD;
	const double mag = (double)r;

	int lastx = (int)round(cos(fstart) * mag);
	int lasty = (int)round(-sin(fstart) * mag);
	for (int i = 1; i <= steps; ++i)
	{
		const double theta = fstart + fstepSz * (double)i;
		int ofsx = (int)round(cos(theta) * mag);
		int ofsy = (int)round(-sin(theta) * mag);

		SDL_RenderLine(rend,
			(float)(x + lastx), (float)(y + lasty),
			(float)(x + ofsx),  (float)(y + ofsy));

		lastx = ofsx;
		lasty = ofsy;
	}
}

void DrawPresent(void)
{
	SDL_RenderPresent(rend);
}
