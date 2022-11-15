#ifndef DRAW_H
#define DRAW_H

#define DISPLAY_SCALE 0.8889

#include "maths.h"
#include <stdint.h>

typedef struct SDL_Window SDL_Window;

// Initialise the drawing subsystem.
//
// Params:
//   window - The application's SDL window.
//
// Returns:
//   0 on success, -1 on failure.
int InitDraw(SDL_Window* window);

// Quit the drawing subsystem.
//
// This is safe (but pointless) to call when the drawing
// subsystem is uninitialised.
void QuitDraw(void);

// Get the actual size of the canvas in pixels.
//
// Returns:
//   size struct with 'w' and 'h' set to the width &
//   height of the canvas in actual pixels.
size GetDrawSizeInPixels(void);

// Set the current draw colour.
//
// Params:
//   c - Colour in the unsigned 32-bit int packed format of:
//       0xFF000000 - red channel
//       0x00FF0000 - blue channel
//       0x0000FF00 - green channel
//       0x000000FF - alpha channel
void SetDrawColour(uint32_t c);

// Clear the entire screen using the current draw colour.
void DrawClear(void);

// Draw a single pixel point at x, y w/ the draw colour.
void DrawPoint(int x, int y);

// Draw rectangle outline.
void DrawRect(int x, int y, int w, int h);

// Draw straight line between x1,y1 and x2,y2.
void DrawLine(int x1, int y1, int x2, int y2);

// Draw outline circle.
void DrawCircle(int x, int y, int r);

// Draw outline circle made of lines w/ a discrete number of steps.
//
// Can be used to draw regular convex polygons such as an octagon.
void DrawCircleSteps(int x, int y, int r, int steps);

// Present the current buffer to the screen.
void DrawPresent(void);

#endif//DRAW_H
