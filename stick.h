#ifndef STICK_H
#define STICK_H

#include "maths.h"
#include "util.h"
#include <stdbool.h>

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

inline void InitDefaults(StickState* p)
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

void DrawAnalogue(const rect* win, StickState* p);
void DrawDigital(const rect* win, StickState* p);

#endif//STICK_H
