#version 410

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 tool_position;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColour;

out vec3 fColour;

void main()
{
	vec4 projected = projection * view * model * vec4(vPosition, 1.0);
	vec4 transformed = view * model * vec4(vPosition, 1.0);

	//fColour = vColour;
	//fColour.r = clamp( abs(distance(tool_position, transformed.xyz)) * 0.2, 0.1, 1.0 );
	fColour = vec3( tool_position.x, vColour.y, vColour.x );

	gl_Position = projected;
}