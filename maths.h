#ifndef MATHS_H
#define MATHS_H

#include <math.h>

#define PI  3.141592653589793238462643383279502884L
#define TAU 6.283185307179586476925286766559005768L

#define RAD2DEG 57.2957795130823208768
#define DEG2RAD 0.01745329251994329577

typedef double vec_t;
typedef struct { vec_t x, y; } vector;

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

#endif//MATHS_H
