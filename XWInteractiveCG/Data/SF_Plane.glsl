#version 330 core

layout (location = 0) out vec4 daColor;

in vec2 texcoord;
in vec4 clipSpacePos;

uniform sampler2D renderTexture;
uniform samplerCube skybox;

void main(){
	// compute normalized device coordinates
	vec2 ndc = (clipSpacePos.xy / clipSpacePos.w)/2.0 + vec2(.5, -.5);
	vec2 reflecTexCoords = vec2(ndc.x, -ndc.y);

	vec4 renderTextureColor = texture(renderTexture, reflecTexCoords);
	//vec4 renderTextureColor = texture(renderTexture, texcoord);
	daColor = renderTextureColor;
}