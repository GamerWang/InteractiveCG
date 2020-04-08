#version 330 core

layout (location = 0) out vec4 daColor;

in vec2 texcoord;
in vec3 worldNormal;
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
uniform samplerCube depthMap;

uniform samplerCube skybox;

float ShadowCalculation(vec3 fragPos){
	float shadow = 0;
	vec3 fragToLight = fragPos - pointLight0pos;
	float closestDepth = texture(depthMap, fragToLight).r;

	closestDepth *= far_plane;

	float currentDepth = length(fragToLight);

	float bias = 0.55;
	shadow = currentDepth - bias > closestDepth ? 1.:0.;

	return shadow;
}

void main(){

	vec3 worldNml = normalize(worldNormal);
	vec3 viewDir = normalize(cameraPosition - worldPosition);
	vec3 reflectDir = reflect(-viewDir, worldNml);

	vec3 pointLight0Dir = normalize(pointLight0pos - worldPosition);
	vec3 pointLight0HalfDir = normalize(viewDir + pointLight0Dir);

	float viewTerm = dot(worldNml, viewDir);
	viewTerm = ceil(viewTerm);
	viewTerm = clamp(viewTerm, 0, 1);

	float shadow = ShadowCalculation(worldPosition);

	// handling pointLight0
	// diffuse part
	vec3 diffuse = vec3(0);
	float pointLight0geoTerm = dot(worldNml, pointLight0Dir);
	//pointLight0geoTerm = clamp(pointLight0geoTerm, 0, 1);
	diffuse += pointLight0geoTerm * pointLight0Intensity;

	// specular part
	vec3 specular = vec3(0);
	float specularTerm = dot(worldNml, pointLight0HalfDir);
	specularTerm = clamp(specularTerm, 0, 1);
	specularTerm = pow(specularTerm, glossiness);

	float specLimit = .1f;
	if(specularTerm > 1 - specLimit){
		specularTerm = 1.f;
	}else{
		specularTerm = 0.f;
	}

	specular += specularTerm * pointLight0Intensity;

	vec3 ambient = ambientLight;

	//diffuse *= (1.-shadow);
	specular *= (1.-shadow);

	if(diffuse.r > -.13f){
		diffuse = vec3(1.f);
	}else if(diffuse.r > -.24f){
		diffuse = vec3(.4f);
	}
	else{
		diffuse = vec3(.34f);
	}

	
	if(brdfMode == 0){
		// sample diffuse texture color
		// vec4 diffuseTextureColor = texture(diffuseTexture, texcoord);
		vec4 diffuseTextureColor = vec4(1., .1, .051, 1.);
		// vec4 specularTextureColor = texture(specularTexture, texcoord);
		vec4 specularTextureColor = vec4(1);
		diffuse *= vec3(diffuseTextureColor);
		specular *= vec3(specularTextureColor);
		ambient *= vec3(diffuseTextureColor);
		daColor = vec4(viewTerm*(diffuse + specular) + ambient, 1.0);
	}else if(brdfMode == 1){
		vec3 reflecColor = texture(skybox, reflectDir).rgb;
		daColor = vec4(diffuse *.6 * (1.-shadow) + specular * (1.-shadow) + reflecColor*.5 + ambient*.5, 1.0);
	}
}