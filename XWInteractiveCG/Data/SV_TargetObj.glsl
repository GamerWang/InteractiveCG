#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texturecoord;

out vec2 texcoord;
out vec3 worldNormal;
out vec3 worldPosition;

layout (std140) uniform Matrices{
	mat4 worldToClamp;
};

uniform mat4 objectToWorldMatrix;
uniform mat3 objectNormalToWorldMatrix;

// standard clip plane value (0, 1, 0, 8) which clips everything below
uniform vec4 clipPlane;

uniform vec3 pointLight0pos;

void main(){

	texcoord = vec2(texturecoord);

	worldNormal = objectNormalToWorldMatrix * normal, 0.0;

	vec4 v = vec4(pos, 1.0);
	v = objectToWorldMatrix * v;

	worldPosition = vec3(v);

	// clip one side of the mirror when render to buffer
	gl_ClipDistance[0] = dot(vec4(worldPosition, 1), clipPlane);

	v = worldToClamp * v;
	gl_Position = v;
}
