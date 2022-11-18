#ifndef METAL_SHADER_TYPES_H
#define METAL_SHADER_TYPES_H

#include <simd/simd.h>

typedef enum
{
	ShaderInputIdxVerticies  = 0,
	ShaderInputViewportScale = 1
} ShaderInputIdx;

typedef struct
{
	vector_float2 position;
	vector_float4 colour;
} ShaderVertex;

#endif//METAL_SHADER_TYPES_H
