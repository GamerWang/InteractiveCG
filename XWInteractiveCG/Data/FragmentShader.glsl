# version 330 core

out layout (location = 0) vec4 daColor;

uniform vec3 dynamicColor;

void main(){
	daColor = vec4(dynamicColor, 1.0);
}