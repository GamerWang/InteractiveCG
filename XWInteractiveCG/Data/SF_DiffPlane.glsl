#version 330 core

layout (location = 0) out vec4 daColor;

in vec2 texcoord;
in vec3 worldPosition;

uniform vec3 worldNormal;
uniform vec3 cameraPosition;

uniform sampler2D diffuseTexture;

void main(){
	vec3 viewDir = normalize(cameraPosition - worldPosition);
	vec3 reflectDir = reflect(-viewDir, worldNormal);

	vec3 diffuseColor = texture(diffuseTexture, texcoord * 3).rgb;

	daColor = vec4(
		diffuseColor,
		1
	);
}