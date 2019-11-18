#version 330 core

layout (location = 0) in vec3 positions;
layout (location = 1) in vec3 normals;
layout (location = 3) in vec2 texCoord;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 MVP;
uniform bool iNormals;

void main()
{	
	//We caclulate the position of the fragment in the View Space and pass it to the fragment shader
	vec4 viewPosition = view*model* vec4(positions, 1.0); 
	FragPos = viewPosition.xyz;

	//TexCoords
	TexCoords = texCoord;

	//Transform the normals into the View Space and pass it to the fragment shader
	mat3 normalMatrix = transpose(inverse(mat3(view*model)));
	Normal = normalMatrix * (iNormals ? -normals : normals);
	
	gl_Position = MVP * vec4(positions, 1.0);
}
