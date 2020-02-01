# version 330 core

out layout (location = 0) vec4 daColor;

in vec3 theNormal;

uniform vec3 dynamicColor;

void main(){
	normalize(theNormal);
	daColor = vec4(theNormal, 1.0);
}