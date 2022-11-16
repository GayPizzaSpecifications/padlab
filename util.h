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
#define GREY1 MKGREY(0x1F, 0xFF)
#define GREY2 MKGREY(0x47, 0xFF)
#define GREY3 MKGREY(0x67, 0xFF)
#define GREY4 MKGREY(0x83, 0xFF)
#define HILIGHT_GR1 0x2F3F1FFF
#define HILIGHT_GR2 0x2F5F2FFF
#define HILIGHT_GR3 0x1FFF1FFF
#define HILIGHT_PU1 0x632E63FF
#define HILIGHT_PU2 0x8A418AFF
#define HILIGHT_PU3 0xFF68FFFF
#define AVATAR 0xFF3333FF

#endif//UTIL_H
