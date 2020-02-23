#ifndef VECTOR_H
#define VECTOR_H

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

#endif//VECTOR_H
