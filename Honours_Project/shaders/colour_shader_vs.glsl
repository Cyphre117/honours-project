#version 410

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 vPosition;
in vec3 vColour;

out vec3 fColour;

void main()
{
	fColour = vColour;
	gl_Position = projection * view * model * vec4(vPosition, 1.0);
}