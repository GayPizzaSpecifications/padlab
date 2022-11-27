#version 330 core

layout (lines) in;
layout (triangle_strip, max_vertices = 8) out;

out vec4 vColour;

uniform vec4 uColour;
uniform vec2 uScaleFact;

const float widthCoef = 1.0;
const float aaCoef = 1.5;

void main()
{
	vec4 from = gl_in[0].gl_Position;
	vec4 to = gl_in[1].gl_Position;
	vec2 normal = normalize(to.xy - from.xy);
	vec4 tangent = vec4(normal.yx * vec2(1.0, -1.0) * uScaleFact, 0.0, 0.0);

	const float cumulCoef = widthCoef + aaCoef;
	vec4 edgeColour = uColour * vec4(1.0, 1.0, 1.0, 0.0);

	vColour = edgeColour;
	gl_Position = from - tangent * cumulCoef;
	EmitVertex();
	gl_Position = to - tangent * cumulCoef;
	EmitVertex();

	vColour = uColour;
	gl_Position = from - tangent * widthCoef;
	EmitVertex();
	gl_Position = to - tangent * widthCoef;
	EmitVertex();

	gl_Position = from + tangent * widthCoef;
	EmitVertex();
	gl_Position = to + tangent * widthCoef;
	EmitVertex();

	vColour = edgeColour;
	gl_Position = from + tangent * cumulCoef;
	EmitVertex();
	gl_Position = to + tangent * cumulCoef;
	EmitVertex();
}
