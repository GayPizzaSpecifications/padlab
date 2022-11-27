#version 330 core

in vec2 inPos;

uniform mat4 uView;

void main()
{
	gl_Position = uView * vec4(inPos, 0.0, 1.0);
}
