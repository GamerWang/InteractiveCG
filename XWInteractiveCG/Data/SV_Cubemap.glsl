#version 330 core

layout (location = 0) in vec3 pos;

out vec3 TexCoords;

layout (std140) uniform Matrices{
	mat4 worldToClamp;
};
uniform mat4 objectToWorldMatrix;

void main() {
	TexCoords = pos;

	vec4 v = vec4(pos, 1.0);
	
	v = objectToWorldMatrix * v;
	v = worldToClamp * v;
	
	gl_Position = v;
}