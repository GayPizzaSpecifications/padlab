#include "maths.h"
#include "draw.h"
#include "stick.h"
#include <SDL.h>
#include <SDL_main.h>
#include <stdio.h>
#include <stdbool.h>

#define CAPTION "PadLab"
#define WINDOW_WIDTH  512
#define WINDOW_HEIGHT 288


int winw = WINDOW_WIDTH, winh = WINDOW_HEIGHT;
size rendSize;

vector plrpos = {10.0, 10.0};
StickState stickl, stickr;

static SDL_Window* window = NULL;
static SDL_JoystickID joyid = -1;
static SDL_Gamepad* pad = NULL;

static Uint64 tickslast;

static bool UseGamepad(SDL_JoystickID aJoyid)
{
	pad = SDL_OpenGamepad(aJoyid);
	if (!pad)
		return false;
	SDL_Log("using gamepad #%d, \"%s\"", aJoyid, SDL_GetGamepadName(pad));
	joyid = aJoyid;
	return true;
}

#define FATAL(CONDITION) if (CONDITION) { return -1; }
int SDL_AppInit(int argc, char* argv[])
{
	FATAL(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) < 0)

	int winflg = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
#ifdef USE_OPENGL
	winflg |= SDL_WINDOW_OPENGL;
#elif defined USE_METAL
	winflg |= SDL_WINDOW_METAL;
#endif
	DrawWindowHints();
	window = SDL_CreateWindow(CAPTION, winw, winh, winflg);
	FATAL(window == NULL)

	FATAL(InitDraw(window))
	SDL_GetWindowSizeInPixels(window, &rendSize.w, &rendSize.h);

	int res = SDL_AddGamepadMappingsFromFile("gamecontrollerdb.txt");
	if (res != -1)
		SDL_Log("read %d mappings from gamecontrollerdb.txt", res);

	int numJoy;
	SDL_JoystickID* joyIds = SDL_GetJoysticks(&numJoy);
	if (joyIds)
	{
		for (int i = 0; i < numJoy; ++i)
			if (SDL_IsGamepad(joyIds[i]) || UseGamepad(joyIds[i]))
				break;
		SDL_free(joyIds);
	}

	InitDefaults(&stickl);
	InitDefaults(&stickr);

	tickslast = SDL_GetTicks();

	return 0;
}

static bool repaint = true;
static bool showavatar = false;
static int side = 0;

int SDL_AppEvent(const SDL_Event* event)
{
	switch (event->type)
	{
	case (SDL_EVENT_KEY_DOWN):
		if (event->key.keysym.sym == SDLK_ESCAPE)
			return 1;
		if (event->key.keysym.sym == SDLK_e)
		{
			showavatar = !showavatar;
			repaint = true;
		}
		return 0;

	case (SDL_EVENT_GAMEPAD_BUTTON_DOWN):
		if (event->gbutton.button == SDL_GAMEPAD_BUTTON_BACK)
		{
			showavatar = !showavatar;
			repaint = true;
		}
		return 0;

	case (SDL_EVENT_QUIT):
		return 1;

	case (SDL_EVENT_WINDOW_RESIZED):
		winw = event->window.data1;
		winh = event->window.data2;
		return 0;

	case (SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED):
		rendSize = (size){event->window.data1, event->window.data2};
		SetDrawViewport(rendSize);
		repaint = true;
		return 0;

	case (SDL_EVENT_WINDOW_EXPOSED):
		repaint = true;
		return 0;

	case (SDL_EVENT_GAMEPAD_AXIS_MOTION):
		if (event->gaxis.which != joyid)
			return 0;
		switch (event->gaxis.axis)
		{
		case (SDL_GAMEPAD_AXIS_LEFTX):
			stickl.rawpos.x = (vec_t)event->gaxis.value / (vec_t)0x7FFF;
			repaint = stickl.recalc = true;
			break;
		case (SDL_GAMEPAD_AXIS_LEFTY):
			stickl.rawpos.y = (vec_t)event->gaxis.value / (vec_t)0x7FFF;
			repaint = stickl.recalc = true;
			break;
		case (SDL_GAMEPAD_AXIS_RIGHTX):
			stickr.rawpos.x = (vec_t)event->gaxis.value / (vec_t)0x7FFF;
			repaint = stickr.recalc = true;
			break;
		case (SDL_GAMEPAD_AXIS_RIGHTY):
			stickr.rawpos.y = (vec_t)event->gaxis.value / (vec_t)0x7FFF;
			repaint = stickr.recalc = true;
			break;
		default:
			break;
		}
		return 0;

	case (SDL_EVENT_MOUSE_BUTTON_DOWN):
		if (event->button.state & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
			side = (event->button.x > winw / 2) ? 1 : 0;
		return 0;

	case (SDL_EVENT_MOUSE_MOTION):
		if (event->motion.state & SDL_BUTTON_LMASK)
		{
			const double hwinw = winw / 2.0;
			const double dispscale = 1.0 / (((hwinw > winh) ? winh : hwinw) * DISPLAY_SCALE / 2.0);
			const vector newpos = {
				CLAMP(((double)event->motion.x - hwinw / 2.0 - hwinw * side) * dispscale, -1.0, 1.0),
				CLAMP(((double)event->motion.y - winh / 2.0) * dispscale, -1.0, 1.0) };

			StickState* stick = side ? &stickr : &stickl;
			stick->rawpos = newpos;
			repaint = stick->recalc = true;
		}
		else if (event->motion.state & SDL_BUTTON_RMASK)
		{
			const double hwinw = winw / 2.0;
			const double valx = SATURATE(1.0 - (double)event->motion.x / (double)hwinw);
			const double valy = SATURATE(1.0 - (double)event->motion.y / (double)winh);

			if (side == 0)
			{
				stickl.digiangle = valx;
				stickl.digideadzone = valy;
				repaint = stickl.recalc = true;
			}
			else
			{
				//p2.accelpow = valy * valy * valy * 32;
				stickr.accelpow = pow(valy * 3, 1.0 + valy * 3);
				repaint = stickr.recalc = true;
			}
		}
		return 0;

	case (SDL_EVENT_GAMEPAD_ADDED):
		if (pad == NULL)
			UseGamepad(event->gdevice.which);
		return 0;

	case (SDL_EVENT_GAMEPAD_REMOVED):
		if (pad != NULL && event->gdevice.which == joyid)
		{
			SDL_CloseGamepad(pad);
			pad = NULL;
			SDL_Log("active gamepad was removed");
		}
		return 0;

	default:
		return 0;
	}
}

int SDL_AppIterate(void)
{
	//FIXME: probably doesn't matter but this isn't very precise
	const Uint64 ticks = SDL_GetTicks();
	const double framedelta = (double)(ticks - tickslast);
	tickslast = ticks;

	if (!repaint)
		return 0;

	// background
	SetDrawColour(GREY1);
	DrawClear();

	const int hrw = rendSize.w / 2;
	DrawDigital(&(rect){ 0, 0, hrw, rendSize.h}, &stickl);
	DrawAnalogue(&(rect){ hrw, 0, hrw, rendSize.h}, &stickr);

	// test player thingo
	if (showavatar)
	{
		const int plrSz = 32;
		const int hplrSz = plrSz / 2;

		plrpos = VecAdd(plrpos, VecScale(stickl.compos, framedelta * 0.5));
		plrpos.x = pfmod(plrpos.x + hplrSz, rendSize.w + plrSz) - hplrSz;
		plrpos.y = pfmod(plrpos.y + hplrSz, rendSize.h + plrSz) - hplrSz;

		SetDrawColour(AVATAR);
		DrawRect(
			(int)plrpos.x - hplrSz,
			(int)plrpos.y - hplrSz,
			plrSz, plrSz);
	}

	DrawPresent();
	repaint = false;
	return 0;
}

void SDL_AppQuit(void)
{
	SDL_CloseGamepad(pad);
	QuitDraw();
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char** argv)
{
	int res = SDL_AppInit(argc, argv);
	while (!res)
	{
		SDL_Event event;
		bool onevent = false;
		if (!showavatar || (stickl.compos.x == 0.0 && stickl.compos.y == 0.0))
		{
			onevent = SDL_WaitEvent(&event) != 0;
		}
		else
		{
			onevent = SDL_PollEvent(&event) > 0;
			repaint = true;
		}
		if (onevent)
		{
			do res = SDL_AppEvent(&event);
			while (!res && SDL_PollEvent(&event) > 0);
		}
		if (!res)
			res = SDL_AppIterate();
	}
	SDL_AppQuit();
	return res;
}
