#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texturecoord;

out vec2 texcoord;
out vec4 clipSpacePos;

layout (std140) uniform Matrices{
	mat4 worldToClamp;
};
uniform mat4 objectToWorldMatrix;

void main(){
	texcoord = vec2(texturecoord);

	vec4 v = vec4(pos, 1.0);
	
	clipSpacePos = worldToClamp * objectToWorldMatrix * v;

	gl_Position = clipSpacePos;
}