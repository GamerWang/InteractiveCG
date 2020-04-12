#version 330 core

layout (location = 0) out vec4 daColor;

in vec2 texcoord;
in vec3 worldNormal;
in vec4 viewNormal;
in vec3 worldPosition;

uniform float glossiness;
//uniform vec3 diffuseColor;
//uniform vec3 specularColor;

uniform vec3 cameraPosition;
uniform float far_plane;

layout (std140) uniform Lights{
	vec3 ambientLight;
	vec3 pointLight0Intensity;
	vec3 pointLight0pos;
};

uniform int brdfMode;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D matcapTexture;
uniform sampler2D matcapMask;
uniform samplerCube depthMap;

uniform samplerCube skybox;

float ShadowCalculation(vec3 fragPos){
	float shadow = 0;
	vec3 fragToLight = fragPos - pointLight0pos;
	float closestDepth = texture(depthMap, fragToLight).r;

	closestDepth *= far_plane;

	float currentDepth = length(fragToLight);

	float bias = 0.25;
	shadow = currentDepth - bias > closestDepth ? 1.:0.;

	return shadow;
}

void main(){

	vec3 worldNml = normalize(worldNormal);
	vec3 viewDir = normalize(cameraPosition - worldPosition);
	vec3 reflectDir = reflect(-viewDir, worldNml);
	vec2 matcapTexcoord = normalize(viewNormal).xy * .5 + vec2(.5, .5);

	vec3 pointLight0Dir = normalize(pointLight0pos - worldPosition);
	vec3 pointLight0HalfDir = normalize(viewDir + pointLight0Dir);

	float viewTerm = dot(worldNml, viewDir);
	float lightTerm = dot(worldNml, pointLight0Dir);

	// cell yin
	float threshHold = -.3;
	float yin = step(threshHold, lightTerm);

	// cell ying
	float shadow = ShadowCalculation(worldPosition);

	float finalDark = (1.-shadow) * yin;

	vec3 brightColor = vec3(1, 1, 1);
	vec3 darkColor = vec3(.6, .3, .3);

	vec3 finalTint = mix(darkColor, brightColor, yin);

	// handling pointLight0
	// diffuse part
	vec3 diffuse = vec3(0);
	float pointLight0geoTerm = dot(worldNml, pointLight0Dir);
	pointLight0geoTerm = clamp(pointLight0geoTerm, 0, 1);
	diffuse += pointLight0geoTerm * pointLight0Intensity;

	// specular part
	vec3 specular = vec3(0);
	float specularTerm = dot(worldNml, pointLight0HalfDir);
	specularTerm = clamp(specularTerm, 0, 1);
	specularTerm = pow(specularTerm, glossiness);

	float specLimit = .3f;
	if(specularTerm > 1 - specLimit){
		specularTerm = 1.f;
	}else{
		specularTerm = 0.f;
	}

	specular += specularTerm * pointLight0Intensity;

	vec3 ambient = ambientLight;

	//diffuse *= (1.-shadow);
	specular *= (1.-shadow);

	// sample diffuse texture color
	vec4 diffuseTextureColor = texture(diffuseTexture, texcoord);
	float matcapMask = texture(matcapMask, texcoord).r;
	vec4 matcapColor = texture(matcapTexture, matcapTexcoord);
	matcapColor = mix(vec4(1), matcapColor * 1.8, matcapMask*.5);

	// vec4 specularTextureColor = texture(specularTexture, texcoord);
	vec4 specularTextureColor = vec4(1);

	diffuse = vec3(diffuseTextureColor * matcapColor);
	specular *= vec3(specularTextureColor);

	daColor = vec4((finalTint * diffuse) + specular, 1.);
	// daColor = vec4(worldNml, 1.);
	// daColor = vec4(vec3(yin), 1.);
	// daColor = vec4(vec3(shadow), 1.);
	// daColor = vec4(vec3(finalDark), 1.);
	// daColor = texture(matcapTexture, texcoord);
	// daColor = vec4(matcapMask);
}