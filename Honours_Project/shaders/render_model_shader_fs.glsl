#version 410
uniform sampler2D tex;

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoord;

out vec4 outColour;

void main()
{
	outColour = vec4(fNormal, 1.0);
	//outColour = texture(tex, fTexCoord);
}