#version 330 core

layout (location = 0) in vec3 positions;
layout (location = 2) in vec3 color;

uniform mat4 MVP;


out vec4 frontColor;

void main()
{
	frontColor = vec4(color, 1.0);

	gl_Position = MVP * vec4(positions, 1.0);
}
