#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

out vec3 worldNormal;
out vec3 worldPosition;

uniform mat4 objectToWorldMatrix;
uniform mat3 objectNormalToWorldMatrix;
uniform mat4 worldToClampMatrix;

uniform vec3 pointLight0pos;

void main(){
//	worldNormal = vec3(objectToWorldMatrix * vec4(normal, 0.0));
	worldNormal = objectNormalToWorldMatrix * normal, 0.0;

	vec4 v = vec4(pos, 1.0);
	v = objectToWorldMatrix * v;

	worldPosition = vec3(v);

	v = worldToClampMatrix * v;
	gl_Position = v;
}
