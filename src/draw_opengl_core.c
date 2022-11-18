#include "draw.h"
#include "maths.h"
#include <GL/gl3w.h>
#include <SDL_video.h>
#include <stdbool.h>

#define OPENGL_VERSION_MAJOR 3
#define OPENGL_VERSION_MINOR 3

static SDL_GLContext* ctx = NULL;
static SDL_Window* window = NULL;
static uint32_t colour    = 0x00000000;
static uint32_t clrColour = 0x00000000;
static bool antialias     = false;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
static void GlErrorCb(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam)
{
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
		printf("%s\n", message);
}
#pragma clang diagnostic pop


void DrawWindowHints(void)
{
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); // Enable MSAA
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8); // 8x MSAA

	// Legacy OpenGL profile
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_VERSION_MAJOR);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_VERSION_MINOR);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
}

int InitDraw(SDL_Window* _window)
{
	window = _window;
	ctx = SDL_GL_CreateContext(window);
	if (ctx == NULL || window == NULL || SDL_GL_MakeCurrent(window, ctx))
	{
		fprintf(stderr, "%s\n", SDL_GetError());
		return -1;
	}

	// Load Core profile extensions
	if (gl3wInit() != GL3W_OK)
	{
		fprintf(stderr, "Failed to init Core profile\n");
		return -1;
	}
	if (!gl3wIsSupported(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR))
	{
		fprintf(stderr, "OpenGL %d.%d unsupported\n", 3, 3);
		return -1;
	}

	// Set debug callback
#if !defined NDEBUG && !defined __APPLE__
	glDebugMessageCallback(GlErrorCb, nullptr);
	glEnable(GL_DEBUG_OUTPUT);
#endif

	SDL_GL_SetSwapInterval(1); // Enable vsync

	// Detect if MSAA is available & active
	int res;
	if (SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &res) == 0 && res == 1)
		if (SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &res) == 0 && res > 0)
			antialias = true;

	return 0;
}

void QuitDraw(void)
{
	SDL_GL_MakeCurrent(window, NULL);
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

void DrawPoint(int x, int y)
{

}

void DrawRect(int x, int y, int w, int h)
{

}

void DrawLine(int x1, int y1, int x2, int y2)
{

}
void DrawCircleSteps(int x, int y, int r, int steps)
{

}

void DrawArcSteps(int x, int y, int r, int startAng, int endAng, int steps)
{

}

void DrawPresent(void)
{
	SDL_GL_SwapWindow(window);
}
