#version 330 core

layout (location = 0) out vec4 daColor;

in vec2 texcoord;
in vec3 worldPosition;

uniform vec3 worldNormal;
uniform vec3 cameraPosition;
uniform float far_plane;

layout (std140) uniform Lights{
	vec3 ambientLight;
	vec3 pointLight0Intensity;
	vec3 pointLight0pos;
};

uniform sampler2D diffuseTexture;
uniform samplerCube depthMap;

float ShadowCalculation(vec3 fragPos){
	float shadow = 0;
	vec3 fragToLight = fragPos - pointLight0pos;
	float closestDepth = texture(depthMap, fragToLight).r;

	closestDepth *= far_plane;

	float currentDepth = length(fragToLight);

	float bias = 2.00;
	shadow = currentDepth - bias > closestDepth ? 1.:0.;

	return shadow;
}

void main(){
	vec3 viewDir = normalize(cameraPosition - worldPosition);
	vec3 reflectDir = reflect(-viewDir, worldNormal);

	vec3 pointLight0Dir = normalize(pointLight0pos - worldPosition);
	vec3 pointLight0HalfDir = normalize(viewDir + pointLight0Dir);

	float viewTerm = dot(worldNormal, viewDir);
	viewTerm = ceil(viewTerm);
	viewTerm = clamp(viewTerm, 0, 1);


	// handling pointLight0
	// diffuse part
	vec3 diffuse = vec3(0);
	float pointLight0geoTerm = dot(worldNormal, pointLight0Dir);
	pointLight0geoTerm = clamp(pointLight0geoTerm, 0, 1);
	diffuse += pointLight0geoTerm * pointLight0Intensity;

	vec3 ambient = ambientLight;

	vec3 diffuseColor = texture(diffuseTexture, texcoord * 3).rgb;

	diffuse *= diffuseColor;
	ambient *= diffuseColor;
	
	// handling shadows
	float shadow = ShadowCalculation(worldPosition);

	daColor = vec4(
		diffuse * (1.-shadow) + ambient,
		1
	);
}