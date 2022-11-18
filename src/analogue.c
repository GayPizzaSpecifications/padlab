#include "maths.h"
#include "draw.h"
#include "stick.h"
#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>

#define CAPTION "PadLab"
#define WINDOW_WIDTH  512
#define WINDOW_HEIGHT 288

static SDL_Window* window = NULL;
static SDL_JoystickID joyid = -1;
static SDL_GameController* pad = NULL;

static bool UseGamepad(int aJoyid)
{
	pad = SDL_GameControllerOpen(aJoyid);
	joyid = SDL_JoystickGetDeviceInstanceID(aJoyid);
	if (pad != NULL)
	{
		printf("using gamepad #%d, \"%s\"\n", aJoyid, SDL_GameControllerName(pad));
		return true;
	}
	return false;
}

#define FATAL(CONDITION, RETURN) if (CONDITION) { res = (RETURN); goto error; }
int main(int argc, char** argv)
{
	int res;
	
	res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
	if (res < 0)
		goto error;

	const int winpos = SDL_WINDOWPOS_CENTERED;
#ifdef USE_OPENGL
	const int winflg = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
#else
	const int winflg = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
#endif
	int winw = WINDOW_WIDTH;
	int winh = WINDOW_HEIGHT;
	DrawWindowHints();
	window = SDL_CreateWindow(CAPTION, winpos, winpos, winw, winh, winflg);
	FATAL(window == NULL, -1)

	FATAL(InitDraw(window), -1)
	size rendSize = GetDrawSizeInPixels();

	if ((res = SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt")) != -1)
		printf("read %d mappings from gamecontrollerdb.txt\n", res);
	for (int i = 0; i < SDL_NumJoysticks(); ++i)
	{
		if (SDL_IsGameController(i))
		{
			if (UseGamepad(i))
				break;
		}
	}

	vector plrpos = {10.0, 10.0};
	StickState stickl, stickr;
	InitDefaults(&stickl);
	InitDefaults(&stickr);

	bool running = true;
	bool repaint = true;
	bool showavatar = false;
	uint32_t tickslast = SDL_GetTicks();
	int side = 0;

	while (running)
	{
		//FIXME: probably doesn't matter but this isn't very precise
		const uint32_t ticks = SDL_GetTicks();
		const double framedelta = (double)(ticks - tickslast);
		tickslast = ticks;
		
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
			do
			{
				switch (event.type)
				{
				case (SDL_KEYDOWN):
					if (event.key.keysym.sym == SDLK_ESCAPE)
					{
						running = false;
					}
					else if (event.key.keysym.sym == SDLK_e)
					{
						showavatar = !showavatar;
						repaint = true;
					}
					break;

				case (SDL_CONTROLLERBUTTONDOWN):
					if (event.cbutton.button == SDL_CONTROLLER_BUTTON_BACK)
					{
						showavatar = !showavatar;
						repaint = true;
					}
					break;
				
				case (SDL_QUIT):
					running = false;
					break;
					
				case (SDL_WINDOWEVENT):
					if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
					{
						winw = event.window.data1;
						winh = event.window.data2;
						rendSize = GetDrawSizeInPixels();
						SetDrawViewport(rendSize);
						repaint = true;
					}
					else if (event.window.event == SDL_WINDOWEVENT_EXPOSED)
					{
						repaint = true;
					}
					break;
					
				case (SDL_CONTROLLERAXISMOTION):
					if (event.caxis.which == joyid)
					{
						if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
						{
							stickl.rawpos.x = (vec_t)event.caxis.value / (vec_t)0x7FFF;
							repaint = stickl.recalc = true;
						}
						else if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
						{
							stickl.rawpos.y = (vec_t)event.caxis.value / (vec_t)0x7FFF;
							repaint = stickl.recalc = true;
						}
						else if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX)
						{
							stickr.rawpos.x = (vec_t)event.caxis.value / (vec_t)0x7FFF;
							repaint = stickr.recalc = true;
						}
						else if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
						{
							stickr.rawpos.y = (vec_t)event.caxis.value / (vec_t)0x7FFF;
							repaint = stickr.recalc = true;
						}
					}
					break;

				case (SDL_MOUSEBUTTONDOWN):
					if (event.button.state & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK ))
					{
						side = (event.button.x > winw / 2) ? 1 : 0;
					}
					break;

				case (SDL_MOUSEMOTION):
					if (event.motion.state & SDL_BUTTON_LMASK)
					{
						const double hwinw = winw / 2.0;
						const double dispscale = 1.0 / (((hwinw > winh) ? winh : hwinw) * DISPLAY_SCALE / 2.0);
						const vector newpos = {
							CLAMP(((double)event.motion.x - hwinw / 2.0 - hwinw * side) * dispscale, -1.0, 1.0),
							CLAMP(((double)event.motion.y - winh / 2.0) * dispscale, -1.0, 1.0) };

						StickState* stick = side ? &stickr : &stickl;
						stick->rawpos = newpos;
						repaint = stick->recalc = true;
					}
					else if (event.motion.state & SDL_BUTTON_RMASK)
					{
						const double hwinw = winw / 2.0;
						const double valx = SATURATE(1.0 - (double)event.motion.x / (double)hwinw);
						const double valy = SATURATE(1.0 - (double)event.motion.y / (double)winh);

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
					
				case (SDL_CONTROLLERDEVICEADDED):
					if (pad == NULL)
						UseGamepad(event.cdevice.which);
					break;
				case (SDL_CONTROLLERDEVICEREMOVED):
					if (pad != NULL && event.cdevice.which == joyid)
					{
						SDL_GameControllerClose(pad);
						pad = NULL;
						printf("active gamepad was removed\n");
					}
					break;
					
				default:
					break;
				}
			}
			while (SDL_PollEvent(&event) > 0);
		}

		if (repaint)
		{
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
		}
	}

	res = 0;
error:
	SDL_GameControllerClose(pad);
	QuitDraw();
	SDL_DestroyWindow(window);
	SDL_Quit();
	return res;
}
