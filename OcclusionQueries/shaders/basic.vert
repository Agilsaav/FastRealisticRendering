#version 330 core

layout (location = 0) in vec3 positions;
layout (location = 2) in vec3 color;

uniform mat4 MVP;
uniform bool OQ;
uniform vec3 BBMin;
uniform vec3 BBMax;

out vec4 frontColor;

void main()
{
	frontColor = vec4(color, 1.0);

	if(OQ)
	{
		mat4 scale = mat4(vec4(BBMax.x-BBMin.x, 0, 0, 0),   
				          vec4(0, BBMax.y-BBMin.y, 0, 0),
				          vec4(0, 0, BBMax.z-BBMin.z, 0),
				          vec4(0, 0, 0, 1));
		  vec4 BC = vec4((BBMax+BBMin)/2, 0);                 
		  vec4 V = scale*vec4(positions-vec3(0.5), 1);            
		  gl_Position = MVP*(BC+V);       
	}
	else
	{
		gl_Position = MVP * vec4(positions, 1.0);
	}
}
