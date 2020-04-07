#version 430 core

layout (location = 0) out vec4 daColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main(){
	daColor = texture(skybox, TexCoords);
}