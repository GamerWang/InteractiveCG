#version 330 core

in layout (location = 0) vec3 pos;
in layout (location = 1) vec3 normal;

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
