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

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

struct Light {
	vec3 Position;
	vec3 Color;

	float linear;
	float quadratic;
};

uniform Light light;

void main() 
{
	//GBuffer data
	vec3 FragPos = texture(gPosition, TexCoords).rgb;
	vec3 Normal = texture(gNormal, TexCoords).rgb;
	vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;

	float AO = texture(ssao, TexCoords).r;

	//Calculate Lighting using a Blinn-Phong shading model
	vec3 ambient = vec3(0.3 * Diffuse * AO); // Add Ambient Occlusion factor!
	//vec3 ambient = vec3(0.3 * Diffuse ); // Add Ambient Occlusion factor!
	vec3 lighting = ambient;
	vec3 viewDir = normalize(-FragPos); // viewpos is (0.0.0) in view-space

	// diffuse
	vec3 lightDir = normalize(light.Position - FragPos);
	vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.Color;

	// specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
	vec3 specular = light.Color * spec;

	// attenuation
	float dist = length(light.Position - FragPos);
	float attenuation = 1.0 / (1.0 + light.linear * dist + light.quadratic * dist * dist);
	diffuse *= attenuation;
	specular *= attenuation;
	lighting += diffuse + specular;

	FragColor = vec4(lighting, 1.0);

	//FragColor = vec4(AO,AO,AO,1.0);
	//FragColor = vec4(0.2, 0.3, 0.7,1.0);
}
