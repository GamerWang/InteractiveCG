#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texturecoord;

out vec3 worldPosition;
out vec4 clipSpacePos;

layout (std140) uniform Matrices{
	mat4 worldToClamp;
};
uniform mat4 objectToWorldMatrix;

void main(){
	vec4 v = vec4(pos, 1.0);
	v = objectToWorldMatrix * v;

	worldPosition = vec3(v);

	clipSpacePos = worldToClamp * v;

	gl_Position = clipSpacePos;
}