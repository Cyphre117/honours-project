#version 410

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColour;

out vec3 fColour;

void main()
{
	fColour = vColour;
	gl_Position = projection * view * model * vec4(vPosition, 1.0);
}