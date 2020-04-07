#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texturecoord;

out vec2 texcoord;
out vec3 worldPosition;
out vec4 viewNormal;

layout (std140) uniform Matrices{
	mat4 worldToClamp; // world to projection
	mat4 worldToView;
	mat4 viewToProj;
};
uniform mat4 objectToWorldMatrix;

void main(){
	texcoord = texturecoord;

	vec4 v = vec4(pos, 1.0);
	v = objectToWorldMatrix * v;

	worldPosition = vec3(v);
	viewNormal = viewToProj * worldToView * vec4(0, 1, 0, 0);

	gl_Position = viewToProj * worldToView * v;
}