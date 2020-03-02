#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texturecoord;

out vec2 texcoord;
out vec3 worldPosition;

layout (std140) uniform Matrices{
	mat4 worldToClamp;
};
uniform mat4 objectToWorldMatrix;

void main(){
	texcoord = texturecoord;

	vec4 v = vec4(pos, 1.0);
	v = objectToWorldMatrix * v;

	worldPosition = vec3(v);

	gl_Position = worldToClamp * v;
}