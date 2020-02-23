#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define CAPTIION "Analogue"
#define WINDOW_WIDTH  512
#define WINDOW_HEIGHT 288

#define DISPLAY_SCALE  0.8889

SDL_Window* window = NULL;
SDL_Renderer* rend = NULL;
SDL_JoystickID joyid = -1;
SDL_GameController* pad = NULL;

typedef double vec_t;
typedef struct {vec_t x, y;} vector;

static inline vector VecAdd(vector l, vector r)
{
	return (vector){l.x + r.x, l.y + r.y};
}

static inline vector VecScale(vector v, vec_t x)
{
	return (vector){v.x * x, v.y * x};
}

static inline double pfmod(double x, double d)
{
	return fmod(fmod(x, d) + d, (d));
}

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

void DrawCircle(int x, int y, int r, int steps)
{
	double stepsz = (M_PI * 2.0) / steps;
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

vector DigitalEight(vector v, double deadzone)
{
	vector res = {0, 0};
	
	if (fabs(v.x) > fabs(v.y * 2.0))
	{
		if (fabs(v.x) > deadzone)
			res.x = copysign(1.0, v.x);
	}
	else if (fabs(v.y) > fabs(v.x * 2.0))
	{
		if (fabs(v.y) > deadzone)
			res.y = copysign(1.0, v.y);
	}
	else if (fabs(v.x) + fabs(v.y) > deadzone * 1.5)
	{
		const double dscale = 1/sqrt(2);
		res.x = copysign(dscale, v.x);
		res.y = copysign(dscale, v.y);
	}
	
	return res;
}

static inline double AccelCurve(double x, double y)
{
	return (x * (x + y)) / (1.0 + y);
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

int main(int argc, char** argv)
{
	int res;
	
	res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
	if (res < 0)
		goto error;
	
	const int winpos = SDL_WINDOWPOS_CENTERED;
	int winw = WINDOW_WIDTH;
	int winh = WINDOW_HEIGHT;
	window = SDL_CreateWindow(CAPTIION, winpos, winpos, winw, winh, SDL_WINDOW_RESIZABLE);
	if (window == NULL)
	{
		res = -1;
		goto error;
	}
	
	rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (rend == NULL)
	{
		res = -1;
		goto error;
	}

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
	vector rawpos = {0.0, 0.0};
	vector compos = {0.0, 0.0};
	
	double preaccel = 0.0, postacel = 0.0;
	double accelpow = 1.25;
	double deadzone = 0.125;
	
	bool running = true;
	bool recalcs = true;
	bool repaint = true;
	bool avatare = false;
	uint32_t tickslast = SDL_GetTicks();
	
	while (running)
	{
		const uint32_t ticks = SDL_GetTicks();
		double framedelta = (double)(ticks - tickslast);
		tickslast = ticks;
		
		SDL_Event event;
		bool onevent = false;
		if (!avatare || (compos.x == 0.0 && compos.y == 0.0))
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
						avatare = !avatare;
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
						if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX)
						{
							rawpos.x = (vec_t)event.caxis.value / (vec_t)0x7FFF;
							repaint = recalcs = true;
						}
						else if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
						{
							rawpos.y = (vec_t)event.caxis.value / (vec_t)0x7FFF;
							repaint = recalcs = true;
						}
					}
					break;
				case (SDL_MOUSEMOTION):
					if (event.motion.state & SDL_BUTTON_LMASK)
					{
						rawpos.x = ((double)event.motion.x - winw / 2.0) / (winh * DISPLAY_SCALE / 2.0);
						rawpos.y = ((double)event.motion.y - winh / 2.0) / (winh * DISPLAY_SCALE / 2.0);
						repaint = recalcs = true;
					}
					else if (event.motion.state & SDL_BUTTON_RMASK)
					{
						double scale = 1.0 - ((double)event.motion.y / (double)winh);
						//accelpow = scale * scale * scale * 32;
						accelpow = pow(scale * 3, 1.0 + scale * 3);
						repaint = recalcs = true;
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
		
		if (recalcs)
		{
			compos = RadialDeadzone(rawpos, deadzone, 0.99);
			//compos = DigitalEight(rawpos, 0.5);
			preaccel = sqrt(compos.x * compos.x + compos.y * compos.y);
			compos = ApplyAcceleration(compos, accelpow);
			postacel = sqrt(compos.x * compos.x + compos.y * compos.y);
			recalcs = false;
		}
		
		if (repaint)
		{
			SDL_Rect rect;
			double size = (double)(winw > winh ? winh : winw) * DISPLAY_SCALE;
			
			// background
			SDL_SetRenderDrawColor(rend, 0x1F, 0x1F, 0x1F, 0xFF);
			SDL_RenderClear(rend);
			
			// test player thingo
			if (avatare)
			{
				plrpos = VecAdd(plrpos, VecScale(compos, framedelta * 0.5));
				plrpos.x = pfmod(plrpos.x, winw);
				plrpos.y = pfmod(plrpos.y, winh);
				
				SDL_SetRenderDrawColor(rend, 0xFF, 0x00, 0x00, 0xFF);
				rect.w = rect.h = 32;
				rect.x = (int)plrpos.x - rect.w / 2;
				rect.y = (int)plrpos.y - rect.h / 2;
				SDL_RenderDrawRect(rend, &rect);
			}
			
			// range rect
			SDL_SetRenderDrawColor(rend, 0x3F, 0x3F, 0x3F, 0xFF);
			rect.w = rect.h = (int)round(size);
			rect.x = (winw - rect.w) / 2;
			rect.y = (winh - rect.h) / 2;
			SDL_RenderDrawRect(rend, &rect);
			
			// acceleration curve
			SDL_SetRenderDrawColor(rend, 0x4F, 0x4F, 0x4F, 0xFF);
			const int accelsamp = (int)(sqrt(size) * 4.20);
			double step = 1.0 / (double)accelsamp;
			double y1 = AccelCurve(0.0, accelpow);
			for (int i = 1; i <= accelsamp; ++i)
			{
				double y2 = AccelCurve(step * i, accelpow);
				SDL_RenderDrawLine(rend,
					(int)(step * (i - 1) * size) + (winw - (int)round(size)) / 2,
					(int)((1.0 - y1) * size) + (winh - (int)round(size)) / 2,
					(int)(step * i * size) + (winw - (int)round(size)) / 2,
					(int)((1.0 - y2) * size) + (winh - (int)round(size)) / 2);
				y1 = y2;
			}
			int tickerx = (int)((preaccel - 0.5) * size);
			int tickery = (int)((0.5 - postacel) * size);
			SDL_SetRenderDrawColor(rend, 0x2F, 0x3F, 0x1F, 0xFF);
			SDL_RenderDrawLine(rend,
				winw / 2 + tickerx,
				(winh - (int)round(size)) / 2,
				winw / 2 + tickerx,
				(winh + (int)round(size)) / 2);
			SDL_SetRenderDrawColor(rend, 0x2F, 0x5F, 0x2F, 0xFF);
			SDL_RenderDrawLine(rend,
				(winw - (int)round(size)) / 2,
				winh / 2 + tickery,
				(winw + (int)round(size)) / 2,
				winh / 2 + tickery);
			
			// guide circle
			SDL_SetRenderDrawColor(rend, 0x4F, 0x4F, 0x4F, 0xFF);
			DrawCircle(winw / 2, winh / 2, (int)round(size) / 2, (int)(sqrt(size) * 8.0));
			
			SDL_SetRenderDrawColor(rend, 0x3F, 0x3F, 0x3F, 0xFF);
			DrawCircle(winw / 2, winh / 2, (int)round(deadzone * size) / 2, (int)(sqrt(size) * 2.0));
			
			// 0,0 line axis'
			SDL_SetRenderDrawColor(rend, 0x2F, 0x2F, 0x2F, 0xFF);
			SDL_RenderDrawLine(rend, 0, winh / 2, winw, winh / 2);
			SDL_RenderDrawLine(rend, winw / 2, 0, winw / 2, winh);
			
			// compensated position
			SDL_SetRenderDrawColor(rend, 0x1F, 0xFF, 0x1F, 0xFF);
			DrawCircle(
				winw / 2 + (int)round(compos.x * size / 2.0),
				winh / 2 + (int)round(compos.y * size / 2.0),
				4, 8);
			
			// raw position
			SDL_SetRenderDrawColor(rend, 0xFF, 0xFF, 0xFF, 0xFF);
			DrawCircle(
				winw / 2 + (int)round(rawpos.x * size / 2.0),
				winh / 2 + (int)round(rawpos.y * size / 2.0),
				8, 16);
			
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
