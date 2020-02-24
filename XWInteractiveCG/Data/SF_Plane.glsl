#version 330 core

layout (location = 0) out vec4 daColor;

in vec2 texcoord;

uniform sampler2D renderTexture;
uniform samplerCube skybox;

void main(){
//	vec4 renderTextureColor = texture(renderTexture, texcoord);
//	daColor = renderTextureColor + vec4(0.1);
	daColor = vec4(0.5);
}