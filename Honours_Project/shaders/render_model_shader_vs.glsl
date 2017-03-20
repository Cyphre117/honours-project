#version 410

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

out vec3 fPositon;
out vec3 fNormal;
out vec2 fTexCoord;

void main()
{
	fPositon = vPosition;
	fNormal = vNormal;
	fTexCoord = vTexCoord;
	gl_Position = projection * view * model * vec4(vPosition, 1.0);
}