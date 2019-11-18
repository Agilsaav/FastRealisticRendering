#shader vertex
#version 330 core

layout(location = 0) in vec3 positions;
layout(location = 1) in vec3 normals;
layout(location = 2) in vec2 texCoord;

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
	vec4 viewPosition = view * model* vec4(positions, 1.0);
	FragPos = viewPosition.xyz;

	//TexCoords
	TexCoords = texCoord;

	//Transform the normals into the View Space and pass it to the fragment shader
	mat3 normalMatrix = transpose(inverse(mat3(view*model)));
	Normal = normalMatrix * (iNormals ? -normals : normals);

	gl_Position = MVP * vec4(positions, 1.0);
}

#shader fragment
#version 330 core

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

void main() {

	//We will store in the g-buffer the fragment position, the normal and the diffuse color.
	gPosition = FragPos;
	gNormal = normalize(Normal);
	gAlbedo.rgb = vec3(0.95); //Diffuse color vector is known as Albedo
}