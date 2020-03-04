#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texturecoord;

out vec2 texcoord;

layout (std140) uniform Matrices{
	mat4 worldToClamp;
};

void main(){
	texcoord = texturecoord;

	vec4 v = vec4(pos, 1.0);
	gl_Position =  v;
}