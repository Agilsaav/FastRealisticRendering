#version 330 core

layout (location = 0) in vec3 positions;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;

void main()
{
	TexCoords = texCoords;

	gl_Position = vec4(positions, 1.0);
}
