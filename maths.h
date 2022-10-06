#ifndef MATHS_H
#define MATHS_H

#include <math.h>

#define PI  3.141592653589793238462643383279502884L
#define TAU 6.283185307179586476925286766559005768L

#define MAX(LHS, RHS) ((LHS > RHS) ? (LHS) : (RHS))
#define MIN(LHS, RHS) ((LHS < RHS) ? (LHS) : (RHS))
#define CLAMP(X, A, B) (MIN(B, MAX(A, X)))
#define SATURATE(X) (CLAMP(X, 0, 1))

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

static inline double AccelCurve(double x, double y)
{
	return (x * (x + y)) / (1.0 + y);
}

#endif//MATHS_H
