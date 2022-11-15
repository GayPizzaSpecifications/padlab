#include "draw.h"
#include "maths.h"
#include <SDL_video.h>
#include <SDL_opengl.h>
#include <stdbool.h>

static SDL_GLContext* ctx = NULL;
static SDL_Window* window = NULL;
static uint32_t colour    = 0x00000000;
static uint32_t clrColour = 0x00000000;
static double scaleWidth  = 0.0;
static double scaleHeight = 0.0;
static bool antialias     = false;

void DrawWindowHints(void)
{
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); // Enable MSAA
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8); // 8x MSAA

	// Legacy OpenGL profile
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
}

int InitDraw(SDL_Window* w)
{
	ctx = SDL_GL_CreateContext(w);
	window = w;
	if (ctx == NULL || window == NULL)
		return -1;

	SDL_GL_SetSwapInterval(1); // Enable vsync

	// Detect if MSAA is available & active
	int res;
	if (SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &res) == 0 && res == 1)
		if (SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &res) == 0 && res > 0)
			antialias = true;

	// Disable depth test & culling, enable MSAA
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);

	SetDrawViewport(GetDrawSizeInPixels()); // Fills scaleWidth & scaleHeight

	// Setup orthographic viewport
	glMatrixMode(GL_PROJECTION);
	glOrtho(0.0, 1.0f, 1.0f, 0.0, 1.0, -1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	return 0;
}

void QuitDraw(void)
{
	SDL_GL_DeleteContext(ctx);
	ctx = NULL;
	window = NULL;
}


size GetDrawSizeInPixels(void)
{
	size out;
	SDL_GL_GetDrawableSize(SDL_GL_GetCurrentWindow(), &out.w, &out.h);
	return out;
}

void SetDrawViewport(size size)
{
	glViewport(0, 0, size.w, size.h);
	scaleWidth  = 1.0 / (double)size.w;
	scaleHeight = 1.0 / (double)size.h;
}


void SetDrawColour(uint32_t c)
{
	colour = c;
}

void DrawClear(void)
{
	if (clrColour != colour)
	{
		const float mul = 1.0f / 255.0f;
		glClearColor(
			(GLclampf)((colour & 0xFF000000) >> 24) * mul,
			(GLclampf)((colour & 0x00FF0000) >> 16) * mul,
			(GLclampf)((colour & 0x0000FF00) >>  8) * mul,
			(GLclampf)((colour & 0x000000FF)) * mul);
		clrColour = colour;
	}
	glClear(GL_COLOR_BUFFER_BIT);
}

static inline void GlColour(void)
{
	const float mul = 1.0f / 255.0f;
	glColor4f(
		(GLclampf)((colour & 0xFF000000) >> 24) * mul,
		(GLclampf)((colour & 0x00FF0000) >> 16) * mul,
		(GLclampf)((colour & 0x0000FF00) >>  8) * mul,
		(GLclampf)((colour & 0x000000FF)) * mul);
}

void DrawPoint(int x, int y)
{
	const double fx = ((double)x) * scaleWidth;
	const double fy = ((double)y) * scaleHeight;

	glBegin(GL_POINTS);
		GlColour();
		glVertex2d(fx, fy);
	glEnd();
}

void DrawRect(int x, int y, int w, int h)
{
	const double fx = (double)x * scaleWidth;
	const double fy = (double)y * scaleHeight;
	const double fw = (double)w * scaleWidth;
	const double fh = (double)h * scaleHeight;

	glBegin(GL_LINE_LOOP);
		GlColour();
		glVertex2d(fx, fy);
		glVertex2d(fx + fw, fy);
		glVertex2d(fx + fw, fy + fh);
		glVertex2d(fx, fy + fh);
	glEnd();
}

void DrawLine(int x1, int y1, int x2, int y2)
{
	const double fx1 = (double)x1 * scaleWidth;
	const double fy1 = (double)y1 * scaleHeight;
	const double fx2 = (double)x2 * scaleWidth;
	const double fy2 = (double)y2 * scaleHeight;

	glBegin(GL_LINES);
		GlColour();
		glVertex2d(fx1, fy1);
		glVertex2d(fx2, fy2);
	glEnd();
}

void DrawCircle(int x, int y, int r)
{
	DrawCircleSteps(x, y, r, (int)(sqrt((double)r) * 8.0));
}

void DrawCircleSteps(int x, int y, int r, int steps)
{
	// Circles look better when offset negatively by half a pixel w/o MSAA
	const double fx = (antialias ? (double)x : (double)x - 0.5f) * scaleWidth;
	const double fy = (antialias ? (double)y : (double)y - 0.5f) * scaleHeight;

	const double stepsz = (double)TAU / (double)steps;
	const double magw   = (double)r * scaleWidth;
	const double magh   = (double)r * scaleHeight;

	glBegin(GL_LINE_LOOP);
		GlColour();
		glVertex2d(fx + magw, fy);
	for (int i = 1; i < steps; ++i)
	{
		const double theta = stepsz * (double)i;
		double ofsx = cos(theta) * magw;
		double ofsy = sin(theta) * magh;
		glVertex2d(fx + ofsx, fy - ofsy);
	}
	glEnd();
}

void DrawPresent(void)
{
	SDL_GL_SwapWindow(window);
}
