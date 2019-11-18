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

uniform sampler2D saaoblur_x;
uniform sampler2D saaoblur_y;


void main()
{
	float AOx = texture(saaoblur_x, TexCoords).r;
	float AOy = texture(saaoblur_y, TexCoords).r;

	float AO = (AOx + AOy) / 2.0;

	FragColor = vec4(AO, AO, AO, 1.0);

}
