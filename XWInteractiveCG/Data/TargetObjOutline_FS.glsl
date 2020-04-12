#version 430 core

layout (location = 0) out vec4 daColor;

void main(){
	vec3 outlineColor = vec3(.8, .4, .3);
	daColor = vec4(outlineColor, 1.);
}