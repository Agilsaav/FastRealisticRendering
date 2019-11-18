#version 330 core

out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D noiseText;
uniform vec3 samples[64];
uniform mat4 proj;

//parameters to tweak the effect
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

const vec2 noiseScale = vec2(640.0/4.0, 480.0/4.0); //We have to scale the TexCoords in order to tile with the noiseText

void main()
{
	//Sample the corresponding texture value using the texture function
	vec3 fragPos = texture(gPosition, TexCoords).xyz;
	vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
	vec3 randomVec = normalize(texture(noiseText, TexCoords * noiseScale).xyz);

	//Create a matrix to transform coordinates from tg-space to view space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

	// Iterate over all the kernel samples, transform to view-space and add them to the current fragment position. Finaly compare the fragment position depth with the sample depth in the view space 		position buffer
	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i)
	{
		vec3 sample = TBN * samples[ì];	//Transform to view space
		sample = fragPos + sample * radius;	//Add to the current fragment pos and multiply the sample by radius to increase/decrease the effective SSAO radius

		//Now we transform sample to clip space so we can sample the position/depth value as if we were rendering its position directly to the screen
		vec4 offset = vec4(sample, 1.0f);
		offset = proj * offset;	//Transform to clip space
		offset.xyz /= offset.w;	//Prespective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; //Range 0.0 to 1.0 to sample the position texture

		float sampledepth = texture(gPosition, offset.xyz).z;	//Get depth value of the kernel

		//We introduce rangeCheck to introduce a correction in the depth values of surfaces far behind the tests surface that contribute incorrectly to the occlusion factor
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampledepth));	//The smoothstep function interpolates its third parameter between the first and the second
		occulsion += (sampledepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / kernelSize);
	FragColor = occlusion;
}
