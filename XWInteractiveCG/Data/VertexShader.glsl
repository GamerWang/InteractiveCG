#version 330 core

in layout (location = 0) vec3 pos;

uniform mat4 objectToClampMatrix;

void main(){
	vec4 v = vec4(pos, 1.0);
	v = objectToClampMatrix * v;
	gl_Position = v;
}
