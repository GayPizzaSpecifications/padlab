#ifndef DRAW_H
#define DRAW_H

#define DISPLAY_SCALE 0.8889

#include <stdint.h>

typedef struct SDL_Renderer SDL_Renderer;
extern SDL_Renderer* rend;

void SetDrawColour(uint32_t c);
void DrawClear(void);
void DrawPoint(int x, int y);
void DrawRect(int x, int y, int w, int h);
void DrawLine(int x1, int y1, int x2, int y2);
void DrawCircle(int x, int y, int r);
void DrawCircleSteps(int x, int y, int r, int steps);

#endif//DRAW_H
