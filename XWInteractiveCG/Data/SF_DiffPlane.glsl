#version 430 core

layout (location = 0) out vec4 daColor;

in vec2 texcoord;
in vec3 worldPosition;

uniform vec3 worldNormal;
uniform vec3 tangent;
uniform vec3 cameraPosition;
uniform float far_plane;

layout (std140) uniform Lights{
	vec3 ambientLight;
	vec3 pointLight0Intensity;
	vec3 pointLight0pos;
};

uniform sampler2D normalMap;
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
	vec3 textureNormal = texture(normalMap, vec2(texcoord.x, 1 - texcoord.y)).rgb;
	textureNormal = textureNormal * 2 - 1;
	textureNormal.b = sqrt(1 - dot(textureNormal.rg, textureNormal.rg));

	vec3 bitangent = cross(worldNormal, tangent);

	mat3 tangentToWorld = mat3(tangent, bitangent, worldNormal);

	vec3 nml = tangentToWorld * textureNormal;

	nml = normalize(nml);

	vec3 viewDir = normalize(cameraPosition - worldPosition);
	vec3 reflectDir = reflect(-viewDir, nml);

	vec3 pointLight0Dir = normalize(pointLight0pos - worldPosition);
	vec3 pointLight0HalfDir = normalize(viewDir + pointLight0Dir);

	float viewTerm = dot(nml, viewDir);
	viewTerm = ceil(viewTerm);
	viewTerm = clamp(viewTerm, 0, 1);

	// handling pointLight0
	// diffuse part
	vec3 diffuse = vec3(0);
	float pointLight0geoTerm = dot(nml, pointLight0Dir);
	pointLight0geoTerm = clamp(pointLight0geoTerm, 0, 1);
	diffuse += pointLight0geoTerm * pointLight0Intensity;

	// specular part
	vec3 specular = vec3(0);
	float glossiness = 80;
	float specularTerm = dot(nml, pointLight0HalfDir);
	specularTerm = clamp(specularTerm, 0, 1);
	specularTerm = pow(specularTerm, glossiness);
	specular += specularTerm * pointLight0Intensity;

	vec3 ambient = ambientLight;
	vec3 diffuseColor = vec3(.5);
	vec3 specularColor = vec3(1);

	diffuse *= diffuseColor;
	specular *= specularColor;
	ambient *= diffuseColor;
	
	// handling shadows
	float shadow = ShadowCalculation(worldPosition);

	daColor = vec4(
		(diffuse + specular) * (1.-shadow) + ambient,
		1
	);


//	daColor = vec4(nml, 1);
//	daColor = vec4(textureNormal, 1);
}