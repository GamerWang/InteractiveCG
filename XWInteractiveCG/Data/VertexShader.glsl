#version 330 core

in layout (location = 0) vec3 pos;
in layout (location = 1) vec3 normal;

out vec4 theNormal;

uniform mat4 objectToClampMatrix;
uniform mat4 objectToWorldMatrix;

void main(){
	theNormal = vec4(normal, 0.0);
	theNormal = objectToWorldMatrix * theNormal;

	vec4 v = vec4(pos, 1.0);
	v = objectToClampMatrix * v;
	gl_Position = v;
}
