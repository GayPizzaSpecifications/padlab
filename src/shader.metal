#include "metal_shader_types.h"
#include <metal_stdlib>
using namespace metal;

struct FragmentInput
{
	float4 position [[position]];
	float4 colour;
};

vertex FragmentInput vertexMain(
	uint vertexId [[vertex_id]],
	device const ShaderVertex* v [[buffer(ShaderInputIdxVerticies)]],
	constant vector_float2& viewportScale [[buffer(ShaderInputViewportScale)]])
{
	float2 eyePos = v[vertexId].position * viewportScale * float2(2.0f, -2.0f) - float2(1.0f, -1.0f);

	FragmentInput o;
	o.position = float4(eyePos, 0.0f, 1.0f);
	o.colour   = v[vertexId].colour;
	return o;
}

fragment float4 fragmentMain(FragmentInput in [[stage_in]])
{
	return in.colour;
}
