#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define CLAMP(X, A, B) (MIN((B), MAX((A), (X))))
#define SATURATE(X) (CLAMP((X), 0, 1))

typedef struct { int w, h; } size;
typedef struct { int x, y, w, h; } rect;

#define MKGREY(L, A) (uint32_t)( \
	(((L) << 24) & 0xFF000000) | \
	(((L) << 16) & 0x00FF0000) | \
	(((L) <<  8) & 0x0000FF00) | \
	((A) & 0x000000FF))

#define WHITE 0xFFFFFFFF

#endif//UTIL_H
