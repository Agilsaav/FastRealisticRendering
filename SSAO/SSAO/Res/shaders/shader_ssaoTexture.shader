#shader vertex
#version 330 core

layout(location = 0) in vec3 positions;
layout(location = 1) in vec2 texCoords;

out vec2 TexCoords;

void main()
{
	TexCoords = texCoords;

	gl_Position = vec4(positions, 1.0);
}

#shader fragment
#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D ssao;


void main()
{

	float AO = texture(ssao, TexCoords).r;

	FragColor = vec4(AO,AO,AO,1.0);

}
