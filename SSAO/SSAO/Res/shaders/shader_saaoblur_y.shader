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

out float FragColor;

in vec2 TexCoords;

uniform sampler2D saao;

uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
	vec2 tex_offset = 1.0 / textureSize(saao, 0); // gets size of single texel
	float result = texture(saao, TexCoords).r * weight[0]; // current fragment's contribution

	for (int i = 1; i < 5; ++i)
	{
		result += texture(saao, TexCoords + vec2(tex_offset.y * i, 0.0)).r * weight[i];
		result += texture(saao, TexCoords - vec2(tex_offset.y * i, 0.0)).r * weight[i];
	}

	FragColor = result;
}