#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texturecoord;

out vec2 texcoord;

uniform mat4 objectToClampMatrix;

void main(){
	texcoord = vec2(texturecoord);

	vec4 v = vec4(pos, 1.0);
	
	v = objectToClampMatrix * v;

	gl_Position = v;
}