#include "maths.h"
#include "draw.h"
#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>

#define CAPTION "PadLab"
#define WINDOW_WIDTH  512
#define WINDOW_HEIGHT 288

#define DISPLAY_SCALE  0.8889

SDL_Window* window = NULL;
SDL_JoystickID joyid = -1;
SDL_GameController* pad = NULL;

bool UseGamepad(int a_joyid)
{
	pad = SDL_GameControllerOpen(a_joyid);
	joyid = SDL_JoystickGetDeviceInstanceID(a_joyid);
	if (pad != NULL)
	{
		printf("using gamepad #%d, \"%s\"\n", a_joyid, SDL_GameControllerName(pad));
		return true;
	}
	return false;
}

vector RadialDeadzone(vector v, double min, double max)
{
	double mag = sqrt(v.x * v.x + v.y * v.y);
	if (mag == 0.0 || mag < min)
	{
		return (vector){0.0, 0.0};
	}
	else if (mag > max)
	{
		return (vector){v.x / mag, v.y / mag};
	}
	else
	{
		double rescale = (mag - min) / (max - min);
		return (vector){v.x / mag * rescale, v.y / mag * rescale};
	}
}

vector DigitalEight(vector v, double angle, double deadzone)
{
	vector res = {0, 0};

	if (fabs(v.x) * angle > fabs(v.y))
	{
		if (fabs(v.x) > deadzone)
			res.x = copysign(1.0, v.x);
	}
	else if (fabs(v.y) * angle > fabs(v.x))
	{
		if (fabs(v.y) > deadzone)
			res.y = copysign(1.0, v.y);
	}
	else if (fabs(v.x) + fabs(v.y) > deadzone * (1.0 + angle))
	{
		const double dscale = 1/sqrt(2);
		res.x = copysign(dscale, v.x);
		res.y = copysign(dscale, v.y);
	}

	return res;
}

vector ApplyAcceleration(vector v, double y)
{
	double mag = sqrt(v.x * v.x + v.y * v.y);
	if (mag > 0.0)
	{
		double curve = AccelCurve(mag, y);
		return (vector){v.x / mag * curve, v.y / mag * curve};
	}
	else
	{
		return (vector){0.0, 0.0};
	}
}

typedef struct
{
	// common
	vector rawpos, compos;
	bool recalc;

	// analogue
	double preaccel, postacel;
	double accelpow;
	double deadzone;

	// digital
	double digiangle;
	double digideadzone;
} StickState;

void InitDefaults(StickState* p)
{
	p->rawpos = (vector){0.0, 0.0};
	p->compos = (vector){0.0, 0.0};

	p->recalc = true;
	p->preaccel = 0.0;
	p->postacel = 0.0;
	p->accelpow = 1.25;
	p->deadzone = 0.125;

	p->digiangle = sqrt(2.0) - 1.0;
	p->digideadzone = 0.5;
}

void DrawAnalogue(const SDL_Rect* win, StickState* p)
{
	if (p->recalc)
	{
		p->compos = RadialDeadzone(p->rawpos, p->deadzone, 0.99);
		p->preaccel = sqrt(p->compos.x * p->compos.x + p->compos.y * p->compos.y);
		p->compos = ApplyAcceleration(p->compos, p->accelpow);
		p->postacel = sqrt(p->compos.x * p->compos.x + p->compos.y * p->compos.y);

		p->recalc = false;
	}

	double size = (double)(win->w > win->h ? win->h : win->w) * DISPLAY_SCALE;

	// range rect
	SetDrawColour(0x3F3F3FFF);
	const int rectSz = (int)round(size);
	DrawRect(
		win->x + (win->w - rectSz) / 2,
		win->y + (win->h - rectSz) / 2,
		rectSz, rectSz);

	const int ox = win->x + win->w / 2;
	const int oy = win->y + win->h / 2;

	// acceleration curve
	SetDrawColour(0x4F4F4FFF);
	const int accelsamp = (int)(sqrt(size) * 4.20);
	const double step = 1.0 / (double)accelsamp;
	double y1 = AccelCurve(0.0, p->accelpow);
	for (int i = 1; i <= accelsamp; ++i)
	{
		double y2 = AccelCurve(step * i, p->accelpow);
		DrawLine(
			win->x + (int)(step * (i - 1) * size) + (win->w - (int)round(size)) / 2,
			win->y + (int)((1.0 - y1) * size) + (win->h - (int)round(size)) / 2,
			win->x + (int)(step * i * size) + (win->w - (int)round(size)) / 2,
			win->y + (int)((1.0 - y2) * size) + (win->h - (int)round(size)) / 2);
		y1 = y2;
	}
	const int tickerx = (int)((p->preaccel - 0.5) * size);
	const int tickery = (int)((0.5 - p->postacel) * size);
	SetDrawColour(0x2F3F1FFF);
	DrawLine(
		ox + tickerx,
		win->y + (win->h - (int)round(size)) / 2,
		ox + tickerx,
		win->y + (win->h + (int)round(size)) / 2);
	SetDrawColour(0x2F5F2FFF);
	DrawLine(
		win->x + (win->w - (int)round(size)) / 2,
		oy + tickery,
		win->x + (win->w + (int)round(size)) / 2,
		oy + tickery);

	// guide circle
	SetDrawColour(0x4F4F4FFF);
	DrawCircle(ox, oy, (int)round(size) / 2);

	SetDrawColour(0x3F3F3FFF);
	DrawCircle(ox, oy, (int)round(p->deadzone * size) / 2);

	// 0,0 line axis'
	SetDrawColour(0x2F2F2FFF);
	DrawLine(
		win->x, oy,
		win->x + win->w, oy);
	DrawLine(
		ox, win->y,
		ox, win->y + win->h);

	// compensated position
	SetDrawColour(0x1FFF1FFF);
	DrawCircleSteps(
		ox + (int)round(p->compos.x * size / 2.0),
		oy + (int)round(p->compos.y * size / 2.0),
		8, 16);
	DrawPoint(
		ox + (int)round(p->compos.x * size / 2.0),
		oy + (int)round(p->compos.y * size / 2.0));

	// raw position
	SetDrawColour(0xFFFFFFFF);
	DrawLine(
		ox + (int)round(p->rawpos.x * size / 2.0) - 4,
		oy + (int)round(p->rawpos.y * size / 2.0),
		ox + (int)round(p->rawpos.x * size / 2.0) + 4,
		oy + (int)round(p->rawpos.y * size / 2.0));
	DrawLine(
		ox + (int)round(p->rawpos.x * size / 2.0),
		oy + (int)round(p->rawpos.y * size / 2.0) - 4,
		ox + (int)round(p->rawpos.x * size / 2.0),
		oy + (int)round(p->rawpos.y * size / 2.0) + 4);
}

void DrawDigital(const SDL_Rect* win, StickState* p)
{
	if (p->recalc)
	{
		p->compos = DigitalEight(p->rawpos, p->digiangle, p->digideadzone);
		p->recalc = false;
	}

	const double size = (double)(win->w > win->h ? win->h : win->w) * DISPLAY_SCALE;

	// range rect
	SetDrawColour(0x3F3F3FFF);
	const int rectSz = (int)round(size);
	DrawRect(
		win->x + (win->w - rectSz) / 2,
		win->y + (win->h - rectSz) / 2,
		rectSz, rectSz);

	// window centre
	const int ox = win->x + win->w / 2;
	const int oy = win->y + win->h / 2;

	// guide circle
	SetDrawColour(0x4F4F4FFF);
	DrawCircle(ox, oy, (int)round(size) / 2);

	// 0,0 line axis'
	SetDrawColour(0x2F2F2FFF);
	DrawLine(
		win->x, oy,
		win->x + win->w, oy);
	DrawLine(
		ox, win->y,
		ox, win->y + win->h);

	// calcuate points for the zone previews
	const double outerinvmag = 1.0 / sqrt(1.0 + p->digiangle * p->digiangle);
	const int outh = (int)round((size * outerinvmag) / 2.0);
	const int outq = (int)round((size * outerinvmag) / 2.0 * p->digiangle);
	const int innh = (int)round(p->digideadzone * size / 2.0);
	const int innq = (int)round(p->digideadzone * size / 2.0 * p->digiangle);

	SetDrawColour(0x3F3F3FFF);

	// angles preview
	DrawLine(ox - outq, oy - outh, ox - innq, oy - innh);
	DrawLine(ox + outq, oy - outh, ox + innq, oy - innh);
	DrawLine(ox + outh, oy - outq, ox + innh, oy - innq);
	DrawLine(ox + outh, oy + outq, ox + innh, oy + innq);
	DrawLine(ox + outq, oy + outh, ox + innq, oy + innh);
	DrawLine(ox - outq, oy + outh, ox - innq, oy + innh);
	DrawLine(ox - outh, oy + outq, ox - innh, oy + innq);
	DrawLine(ox - outh, oy - outq, ox - innh, oy - innq);

	// deadzone octagon
	DrawLine(ox - innq, oy - innh, ox + innq, oy - innh);
	DrawLine(ox + innq, oy - innh, ox + innh, oy - innq);
	DrawLine(ox + innh, oy - innq, ox + innh, oy - innq);
	DrawLine(ox + innh, oy - innq, ox + innh, oy + innq);
	DrawLine(ox + innh, oy + innq, ox + innq, oy + innh);
	DrawLine(ox + innq, oy + innh, ox - innq, oy + innh);
	DrawLine(ox - innq, oy + innh, ox - innh, oy + innq);
	DrawLine(ox - innh, oy + innq, ox - innh, oy - innq);
	DrawLine(ox - innh, oy - innq, ox - innq, oy - innh);

	// compensated position
	SetDrawColour(0x1FFF1FFF);
	DrawCircleSteps(
		ox + (int)round(p->compos.x * size / 2.0),
		oy + (int)round(p->compos.y * size / 2.0),
		8, 16);
	DrawPoint(
		ox + (int)round(p->compos.x * size / 2.0),
		oy + (int)round(p->compos.y * size / 2.0));

	// raw position
	SetDrawColour(0xFFFFFFFF);
	DrawLine(
		ox + (int)round(p->rawpos.x * size / 2.0) - 4,
		oy + (int)round(p->rawpos.y * size / 2.0),
		ox + (int)round(p->rawpos.x * size / 2.0) + 4,
		oy + (int)round(p->rawpos.y * size / 2.0));
	DrawLine(
		ox + (int)round(p->rawpos.x * size / 2.0),
		oy + (int)round(p->rawpos.y * size / 2.0) - 4,
		ox + (int)round(p->rawpos.x * size / 2.0),
		oy + (int)round(p->rawpos.y * size / 2.0) + 4);
}

int main(int argc, char** argv)
{
	int res;
	
	res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
	if (res < 0)
		goto error;
	
	const int winpos = SDL_WINDOWPOS_CENTERED;
	const int winflg = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
	int winw = WINDOW_WIDTH;
	int winh = WINDOW_HEIGHT;
	window = SDL_CreateWindow(CAPTION, winpos, winpos, winw, winh, winflg);
	if (window == NULL)
	{
		res = -1;
		goto error;
	}

	const int rendflags = SDL_RENDERER_PRESENTVSYNC;
	rend = SDL_CreateRenderer(window, -1, rendflags);
	if (rend == NULL)
	{
		res = -1;
		goto error;
	}

	int rw, rh;
	SDL_GetRendererOutputSize(rend, &rw, &rh);

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
						SDL_GetRendererOutputSize(rend, &rw, &rh);
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
			SetDrawColour(0x1F1F1FFF);
			DrawClear();

			const int hrw = rw / 2;
			DrawDigital(&(SDL_Rect){ 0, 0, hrw, rh }, &stickl);
			DrawAnalogue(&(SDL_Rect){hrw, 0, hrw, rh}, &stickr);

			// test player thingo
			if (showavatar)
			{
				plrpos = VecAdd(plrpos, VecScale(stickl.compos, framedelta * 0.5));
				plrpos.x = pfmod(plrpos.x, rw);
				plrpos.y = pfmod(plrpos.y, rh);

				SetDrawColour(0xFF0000FF);
				const int plrSz = 32;
				DrawRect(
					(int)plrpos.x - plrSz / 2,
					(int)plrpos.y - plrSz / 2,
					plrSz, plrSz);
			}

			SDL_RenderPresent(rend);
			repaint = false;
		}
	}
	
	res = 0;
error:
	SDL_GameControllerClose(pad);
	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return res;
}
