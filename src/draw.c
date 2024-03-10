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
	const float stepsz = (float)TAU / steps;
	const float mag = (float)r;
	float lastx = mag;
	float lasty = 0;
	for (int i = 1; i <= steps; ++i)
	{
		float ofsx = cosf(stepsz * i) * mag;
		float ofsy = sinf(stepsz * i) * mag;

		SDL_RenderLine(rend,
			x + lastx, y + lasty,
			x + ofsx,  y + ofsy);

		lastx = ofsx;
		lasty = ofsy;
	}
}

void DrawArcSteps(int x, int y, int r, int startAng, int endAng, int steps)
{
	const float fstart = (float)startAng * (float)DEG2RAD;
	const float fstepSz = (float)(endAng - startAng) / abs(steps) * (float)DEG2RAD;
	const float mag = (float)r;

	float lastx =  cosf(fstart) * mag;
	float lasty = -sinf(fstart) * mag;
	for (int i = 1; i <= steps; ++i)
	{
		const float theta = fstart + fstepSz * (float)i;
		float ofsx =  cosf(theta) * mag;
		float ofsy = -sinf(theta) * mag;

		SDL_RenderLine(rend,
			x + lastx, y + lasty,
			x + ofsx,  y + ofsy);

		lastx = ofsx;
		lasty = ofsy;
	}
}

void DrawPresent(void)
{
	SDL_RenderPresent(rend);
}
