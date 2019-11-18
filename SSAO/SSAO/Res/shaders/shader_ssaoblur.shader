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

uniform sampler2D ssao;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(ssao, 0)); //textureSize returns a vec2 of the given texture's dimension
	float result = 0.0;

	//Traverse the surrounding SSAO textels between -2.0 and 2.0 sampling the SSAO texture and amount identical to noise texture`s dimension(16)
	for (int x = -2; x < 2; ++x)
	{
		for (int y = -2; y < 2; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(ssao, TexCoords + offset).r; //Offset each texture coord by the exact size of a singe textel
		}
	}

	FragColor = result / (4.0 * 4.0); //Avarage the result 
}