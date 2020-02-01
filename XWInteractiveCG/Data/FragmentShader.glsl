# version 330 core

out layout (location = 0) vec4 daColor;

in vec4 theNormal;

uniform vec3 dynamicColor;

void main(){
	normalize(theNormal);
	daColor = vec4(vec3(theNormal), 1.0);
}