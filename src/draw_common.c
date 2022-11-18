#include "draw.h"
#include "maths.h"
#include <stdlib.h>

void DrawCircle(int x, int y, int r)
{
	const int steps = (int)(sqrt((double)r) * 8.0);
	DrawCircleSteps(x, y, r, steps);
}

void DrawArc(int x, int y, int r, int startAng, int endAng)
{
	const int steps = (int)(sqrt((double)r) * (double)abs(endAng - startAng) / 360.0 * 8.0);
	DrawArcSteps(x, y, r, startAng, endAng, steps);
}
