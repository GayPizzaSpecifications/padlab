#version 330 core

layout (lines) in;
layout (triangle_strip, max_vertices = 8) out;

out vec4 vColour;

uniform vec4 uColour;
uniform vec2 uScaleFact;

void main()
{
	vec2 n = normalize(gl_in[1].gl_Position.xy - gl_in[0].gl_Position.xy);
	vec4 c = vec4(n.yx * vec2(1.0, -1.0) * uScaleFact, 0.0, 0.0);

	const float width = 0.75;
	const float fuzz = 2.0;

	const float widthFuzz = width + fuzz;

	vColour = uColour * vec4(1.0, 1.0, 1.0, 0.0);
	gl_Position = gl_in[0].gl_Position - c * widthFuzz;
	EmitVertex();
	gl_Position = gl_in[1].gl_Position - c * widthFuzz;
	EmitVertex();

	vColour = uColour;
	gl_Position = gl_in[0].gl_Position - c * width;
	EmitVertex();
	gl_Position = gl_in[1].gl_Position - c * width;
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + c * width;
	EmitVertex();
	gl_Position = gl_in[1].gl_Position + c * width;
	EmitVertex();

	vColour = uColour * vec4(1.0, 1.0, 1.0, 0.0);
	gl_Position = gl_in[0].gl_Position + c * widthFuzz;
	EmitVertex();
	gl_Position = gl_in[1].gl_Position + c * widthFuzz;
	EmitVertex();
}
