#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define CLAMP(X, A, B) (MIN((B), MAX((A), (X))))
#define SATURATE(X) (CLAMP((X), 0, 1))

typedef struct { int x, y; } point;
typedef struct { int w, h; } size;
typedef struct { int x, y, w, h; } rect;

#define MKGREY(L, A) (uint32_t)( \
	(((L) << 24) & 0xFF000000) | \
	(((L) << 16) & 0x00FF0000) | \
	(((L) <<  8) & 0x0000FF00) | \
	((A) & 0x000000FF))

#define MKRGB(C) (uint32_t)(((C) << 8) | 0x000000FF)

#define WHITE 0xFFFFFFFF
#define GREY1 MKGREY(0x1F, 0xFF)
#define GREY2 MKGREY(0x37, 0xFF)
#define GREY3 MKGREY(0x4F, 0xFF)
#define GREY4 MKGREY(0x5F, 0xFF)
#define GREY5 MKGREY(0x83, 0xFF)
#define HILIGHT_GR1 MKRGB(0x2F3F1F)
#define HILIGHT_GR2 MKRGB(0x387138)
#define HILIGHT_GR3 MKRGB(0x1FFF1F)
#define HILIGHT_PU1 MKRGB(0x632E63)
#define HILIGHT_PU2 MKRGB(0x8A418A)
#define HILIGHT_PU3 MKRGB(0xFF68FF)
#define AVATAR MKRGB(0xFF3333)

#endif//UTIL_H
