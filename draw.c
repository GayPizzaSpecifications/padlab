#include "draw.h"
#include "maths.h"
#include <SDL_render.h>

SDL_Renderer* rend = NULL;

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
	SDL_RenderDrawPoint(rend, x, y);
}

void DrawRect(int x, int y, int w, int h)
{
	SDL_Rect dst = {
		.x = x, .y = y,
		.w = w, .h = h };
	SDL_RenderDrawRect(rend, &dst);
}

void DrawLine(int x1, int y1, int x2, int y2)
{
	SDL_RenderDrawLine(rend, x1, y1, x2, y2);
}

void DrawCircle(int x, int y, int r)
{
	DrawCircleSteps(x, y, r, (int)(sqrt((double)r) * 8.0));
}

void DrawCircleSteps(int x, int y, int r, int steps)
{
	double stepsz = (double)TAU / steps;
	int lastx = r;
	int lasty = 0;
	for (int i = 0; i <= steps; ++i)
	{
		const double mag = (double)r;
		int ofsx = (int)round(cos(stepsz * i) * mag);
		int ofsy = (int)round(sin(stepsz * i) * mag);

		SDL_RenderDrawLine(rend,
			x + lastx, y + lasty,
			x + ofsx, y + ofsy);

		lastx = ofsx;
		lasty = ofsy;
	}
}