#version 330 core

layout (location = 0) out vec4 daColor;

in vec3 worldPosition;
in vec4 clipSpacePos;

uniform vec3 worldNormal;
uniform vec3 cameraPosition;

uniform sampler2D renderTexture;
uniform samplerCube skybox;

void main(){
	vec3 viewDir = normalize(cameraPosition - worldPosition);
	vec3 reflectDir = reflect(-viewDir, worldNormal);

	// compute normalized device coordinates
	vec2 ndc = (clipSpacePos.xy / clipSpacePos.w)/2.0 + vec2(.5, -.5);
	vec2 reflecTexCoords = vec2(ndc.x, -ndc.y);

	vec3 renderTextureColor = texture(renderTexture, reflecTexCoords).rgb;

	float bias = .01;
	float renderTexWeight = renderTextureColor.x - bias;
	renderTexWeight = max(renderTexWeight, renderTextureColor.y - bias);
	renderTexWeight = max(renderTexWeight, renderTextureColor.z - bias);
	renderTexWeight = ceil(renderTexWeight);
	float selfWeight = 1 - renderTexWeight;


	vec3 envReflectColor = texture(skybox, reflectDir).rgb;

	float reflectionColor = .6;

	daColor = vec4(
		renderTextureColor * renderTexWeight * reflectionColor + 
		envReflectColor * selfWeight,
		1
	);
}