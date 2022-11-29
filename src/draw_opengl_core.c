#include "draw.h"
#include "glslShaders.h"
#include "maths.h"
#include <GL/gl3w.h>
#include <SDL_video.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct { float x, y; } vertex;

enum { ATTRIB_VERTPOS, NUM_ATTRIBS };
static const char* const attribNames[] =
{
	[ATTRIB_VERTPOS] = "inPos",
//	[ATTRIB_COLOUR]  = "inColour"
};


#define OPENGL_VERSION_MAJOR 3
#define OPENGL_VERSION_MINOR 3

static SDL_GLContext* ctx = NULL;
static SDL_Window* window = NULL;
static uint32_t
	colour     = 0x00000000,
	drawColour = 0x00000000,
	clrColour  = 0x00000000;

#define DRAWLIST_MAX_SIZE 480
static vertex drawListVerts[DRAWLIST_MAX_SIZE];
static uint16_t drawListIndices[DRAWLIST_MAX_SIZE];
static uint16_t drawListCount = 0, drawListVertNum = 0;
static GLuint vao = 0, drawListVbo = 0, drawListIbo = 0;

static GLuint program = 0;
static GLint uView, uColour, uScaleFact;


#if DRAWLIST_MAX_SIZE < 2 || DRAWLIST_MAX_SIZE >= UINT16_MAX
 #error DRAWLIST_MAX_SIZE must be larger than 1 and smaller than 65535
#endif

#if !defined NDEBUG && !defined __APPLE__
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
#endif


void DrawWindowHints(void)
{
	// Modern OpenGL profile
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_VERSION_MAJOR);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_VERSION_MINOR);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}

static inline GLuint CompilerShader(const char* src, GLenum type)
{
	GLuint shader = glCreateShader(type);

	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	int res, logLen;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
	if (res != GL_TRUE)
	{
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			char* log = malloc(logLen);
			glGetShaderInfoLog(shader, logLen, NULL, log);
			fprintf(stderr, "Shader type %#06X compilation failed:\n%s\n", type, log);
			free(log);
		}
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

static inline GLuint LinkProgram(
	GLuint vertShader, GLuint geomShader, GLuint fragShader,
	const char* const attrNames[], GLuint attrCount)
{
	GLuint progId = glCreateProgram();

	// Bind attributes
	for (GLuint i = 0; i < attrCount; ++i)
		glBindAttribLocation(progId, i, (const GLchar*)attrNames[i]);

	// Attach shaders & link program
	glAttachShader(progId, vertShader);
	glAttachShader(progId, geomShader);
	glAttachShader(progId, fragShader);
	glLinkProgram(progId);

	int res, logLen;
	glGetProgramiv(progId, GL_LINK_STATUS, &res);
	if (res != GL_TRUE)
	{
		glGetProgramiv(progId, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			char* log = malloc(logLen);
			glGetProgramInfoLog(progId, logLen, NULL, log);
			fprintf(stderr, "Program link failed:\n%s\n", log);
			free(log);
		}
		glDeleteProgram(progId);
		return 0;
	}

	return progId;
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

	SDL_GL_SetSwapInterval(1); // Enable vsync

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
	glDebugMessageCallback(GlErrorCb, NULL);
	glEnable(GL_DEBUG_OUTPUT);
#endif

	// Ensure culling & depth testing are off
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND); // Enable alpha blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	// Compile shaders
	GLuint vert = CompilerShader(vert_glsl, GL_VERTEX_SHADER);
	if (!vert)
		return -1;
	GLuint geom = CompilerShader(geom_glsl, GL_GEOMETRY_SHADER);
	if (!geom)
	{
		glDeleteShader(vert);
		return -1;
	}
	GLuint frag = CompilerShader(frag_glsl, GL_FRAGMENT_SHADER);
	if (!frag)
	{
		glDeleteShader(geom);
		glDeleteShader(vert);
		return -1;
	}

	// Link program
	program = LinkProgram(vert, geom, frag, attribNames, NUM_ATTRIBS);
	glDeleteShader(frag);
	glDeleteShader(geom);
	glDeleteShader(vert);
	if (!program)
		return -1;

	// Get uniforms
	uView = glGetUniformLocation(program, "uView");
	uColour = glGetUniformLocation(program, "uColour");
	uScaleFact = glGetUniformLocation(program, "uScaleFact");

	glUseProgram(program); // Use program

	// Create & bind vertex attribute array
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Setup draw list
	glGenBuffers(1, &drawListVbo);
	glBindBuffer(GL_ARRAY_BUFFER, drawListVbo);
	glGenBuffers(1, &drawListIbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawListIbo);

	glBufferData(GL_ARRAY_BUFFER, DRAWLIST_MAX_SIZE * sizeof(vertex), NULL, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, DRAWLIST_MAX_SIZE * sizeof(uint16_t), NULL, GL_DYNAMIC_DRAW);

	// Setup draw list vertex attributes
	glEnableVertexAttribArray(ATTRIB_VERTPOS);
	glVertexAttribPointer(ATTRIB_VERTPOS, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*)0);

	// Reset viewport & clear
	SetDrawViewport(GetDrawSizeInPixels());
	glUniform4f(uColour, 1.0f, 1.0f, 1.0f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	return 0;
}

void QuitDraw(void)
{
	if (drawListVbo)
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &drawListVbo);
		drawListVbo = 0;
	}

	if (drawListIbo)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &drawListIbo);
		drawListIbo = 0;
	}

	if (vao)
	{
		glDisableVertexAttribArray(ATTRIB_VERTPOS);
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}

	if (program)
	{
		glUseProgram(0);
		glDeleteProgram(program);
		program = 0;
	}

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
	vertex s = (vertex){2.0f / (float)size.w, 2.0f / (float)size.h};
	float mat[16] = {
		  s.x,  0.0f, 0.0f, 0.0f,
		 0.0f,  -s.y, 0.0f, 0.0f,
		 0.0f,  0.0f, 1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f, 1.0f};
	glUniformMatrix4fv(uView, 1, GL_FALSE, mat);
	glUniform2f(uScaleFact, 1.0f / (float)size.w, 1.0f / (float)size.h);
}


static int drawCount = 0;
static void FlushDrawBuffers()
{
	if (!drawListCount)
		return;

	GLsizeiptr vboSize = drawListVertNum * (GLsizeiptr)sizeof(vertex);
	GLsizeiptr iboSize = drawListCount * (GLsizeiptr)sizeof(uint16_t);
	glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)0, vboSize, drawListVerts);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)0, iboSize, drawListIndices);

	glDrawElements(GL_LINES, drawListCount, GL_UNSIGNED_SHORT, (GLvoid*)0);

	drawListVertNum = 0;
	drawListCount = 0;
	++drawCount;
}

static void UnpackColour(GLfloat* out)
{
	const float mul = 1.0f / 255.0f;
	out[0] = (GLfloat)((colour & 0xFF000000) >> 24) * mul;
	out[1] = (GLfloat)((colour & 0x00FF0000) >> 16) * mul;
	out[2] = (GLfloat)((colour & 0x0000FF00) >>  8) * mul;
	out[3] = (GLfloat)((colour & 0x000000FF)) * mul;
}

static void UpdateDrawColour()
{
	FlushDrawBuffers();                   // Flush anything that might still be in the buffer
	GLfloat draw[4]; UnpackColour(draw);  // Convert packed RGBA32 to float
	glUniform4fv(uColour, 1, draw);       // Pass new colour to shader
	drawColour = colour;                  // Mark state as up to date
}


void SetDrawColour(uint32_t c)
{
	colour = c;
}

void DrawClear(void)
{
	if (clrColour != colour)
	{
		GLfloat clear[4]; UnpackColour(clear);
		glClearColor(clear[0], clear[1], clear[2], clear[3]);
		clrColour = colour;
	}
	glClear(GL_COLOR_BUFFER_BIT);
}

void DrawPoint(int x, int y)
{
	DrawCircleSteps(x, y, 1, 4);
}

void DrawRect(int x, int y, int w, int h)
{
#if 8 <= DRAWLIST_MAX_SIZE
	if (drawColour != colour)
		UpdateDrawColour();
	else if (drawListCount > DRAWLIST_MAX_SIZE - 8)
		FlushDrawBuffers();

	uint16_t base = drawListVertNum;
	drawListVerts[drawListVertNum++] = (vertex){(float)x, (float)y};
	drawListVerts[drawListVertNum++] = (vertex){(float)x + (float)w, (float)y};
	drawListVerts[drawListVertNum++] = (vertex){(float)x + (float)w, (float)y + (float)h};
	drawListVerts[drawListVertNum++] = (vertex){(float)x, (float)y + (float)h};
	drawListIndices[drawListCount++] = base;
	drawListIndices[drawListCount++] = base + 1;
	drawListIndices[drawListCount++] = base + 1;
	drawListIndices[drawListCount++] = base + 2;
	drawListIndices[drawListCount++] = base + 2;
	drawListIndices[drawListCount++] = base + 3;
	drawListIndices[drawListCount++] = base + 3;
	drawListIndices[drawListCount++] = base;

	if (drawListCount == DRAWLIST_MAX_SIZE)
		FlushDrawBuffers();
#else
 #pragma message("DRAWLIST_MAX_SIZE is too low, DrawRect functionality disabled")
#endif
}

void DrawLine(int x1, int y1, int x2, int y2)
{
	if (drawColour != colour)
		UpdateDrawColour();
	else if (drawListCount > DRAWLIST_MAX_SIZE - 2)
		FlushDrawBuffers();

	vertex from = {(float)x1, (float)y1}, to = {(float)x2, (float)y2};
	if (drawListVertNum > 0 && memcmp(&from, &drawListVerts[drawListVertNum - 1], sizeof(vertex)) == 0)
	{
		// Reuse last vertex
		drawListIndices[drawListCount++] = drawListVertNum - 1;
	}
	else
	{
		drawListVerts[drawListVertNum] = from;
		drawListIndices[drawListCount++] = drawListVertNum++;
	}
	drawListVerts[drawListVertNum] = to;
	drawListIndices[drawListCount++] = drawListVertNum++;
}

static void ArcSubSlice(
	float x, float y,
	float magx, float magy,
	float angle, float stride,
	int num)
{
	if (!num)
		return;
	drawListVerts[drawListVertNum] = (vertex){
		x + cosf(angle) * magx,
		y - sinf(angle) * magy};
	for (int i = 1; i <= num; ++i)
	{
		const float theta = angle + stride * (float)i;
		float ofsx = cosf(theta) * magx;
		float ofsy = sinf(theta) * magy;

		drawListIndices[drawListCount++] = drawListVertNum++;
		drawListVerts[drawListVertNum] = (vertex){x + ofsx, y - ofsy};
		drawListIndices[drawListCount++] = drawListVertNum;
	}
	drawListVertNum++;
}

static void ArcSlice(
	float x, float y,
	float magx, float magy,
	float angle, float stride,
	int steps)
{
	int i = 0;
	while (true)
	{
		const int subSteps = (MIN(drawListCount + (steps - i) * 2, DRAWLIST_MAX_SIZE) - drawListCount) / 2;
		if (subSteps)
			ArcSubSlice(x, y, magx, magy, angle, stride, subSteps);
		i += subSteps;
		if (i < steps)
			FlushDrawBuffers();
		else
			break;
		angle += stride * (float)subSteps;
	}
}

void DrawCircleSteps(int x, int y, int r, int steps)
{
	if (drawColour != colour)
		UpdateDrawColour();

	const float fx = (float)x, fy = (float)y;
	const float stepSz = (float)TAU / (float)abs(steps);
	const float mag = (float)r;
	// Check if whole circle can fit in the buffer
	if (drawListCount > DRAWLIST_MAX_SIZE - steps * 2)
	{
		// Draw circle as segmented arcs
		ArcSlice(fx, fy, mag, mag, 0.0f, stepSz, steps);
	}
	else
	{
		// Draw whole circle in a single loop
		uint16_t base = drawListVertNum;
		drawListVerts[drawListVertNum] = (vertex){fx + mag, fy};
		drawListIndices[drawListCount++] = drawListVertNum++;
		for (int i = 1; i < steps; ++i)
		{
			const float theta = stepSz * (float)i;
			float ofsx = cosf(theta) * mag;
			float ofsy = sinf(theta) * mag;

			drawListVerts[drawListVertNum] = (vertex){fx + ofsx, fy + ofsy};
			drawListIndices[drawListCount++] = drawListVertNum;
			drawListIndices[drawListCount++] = drawListVertNum++;
		}
		drawListIndices[drawListCount++] = base;
	}
}

void DrawArcSteps(int x, int y, int r, int startAng, int endAng, int steps)
{
	if (drawColour != colour)
		UpdateDrawColour();

	const float mag = (float)r;
	const float fstart = (float)startAng * (float)DEG2RAD;
	const float fstepSz = (float)(endAng - startAng) / (float)abs(steps) * (float)DEG2RAD;
	ArcSlice((float)x, (float)y, mag, mag, fstart, fstepSz, steps);
}

void DrawPresent(void)
{
	FlushDrawBuffers();
	SDL_GL_SwapWindow(window);
#ifndef NDEBUG
	//fprintf(stderr, "%d draw call(s)\n", drawCount);
#endif
	drawCount = 0;
}
