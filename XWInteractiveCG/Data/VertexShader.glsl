#version 330 core

in layout (location = 0) vec3 pos;
in layout (location = 1) vec3 normal;

out vec3 theNormal;

uniform mat4 objectToClampMatrix;

void main(){
	theNormal = normal;

	vec4 v = vec4(pos, 1.0);
	v = objectToClampMatrix * v;
	gl_Position = v;
}
