#version 430 core

layout (location = 0) out vec4 daColor;

uniform vec3 color;

void main(){
	daColor = vec4(color, 1.);
}