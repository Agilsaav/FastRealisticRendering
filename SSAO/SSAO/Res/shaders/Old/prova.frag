#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D noiseText;


uniform vec3 samples[64];

// parameters 
int kernelSize = 1;
float radius = 0.5;
float bias = 0.025;


const vec2 noiseScale = vec2(640.0/4.0, 480.0/4.0); //We have to scale the TexCoords in order to tile with the noiseText

uniform mat4 projection;

void main()
{
    // Sample the corresponding texture value using the texture function
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(noiseText, TexCoords * noiseScale).xyz);

    
    //FragColor = vec4(fragPos, 1.0);  ->PINTA! moure mouse!!!!!
	//FragColor = vec4(normal, 1.0);  ->PINTA! same
	//FragColor = vec4(noiseText, 1.0); ->PINTA!
	vec3 provacol = vec3(1.0, 0.2, 1.0);

	//FragColor = vec4(provacol, 1.0);

	//FragColor = vec4((normal+vec3(1.))*0.5, 1.0);
	if (fragPos.z < 0.0)
		FragColor = vec4(1.0);
	else FragColor = vec4(fragPos.z*100., .4, .4, 1.);
//	FragColor = vec4(fragPos.z, fragPos.z, fragPos.z, 0.0);
	
}


