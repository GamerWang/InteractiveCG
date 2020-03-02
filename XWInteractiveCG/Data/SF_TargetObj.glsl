#version 330 core

layout (location = 0) out vec4 daColor;

in vec2 texcoord;
in vec3 worldNormal;
in vec3 worldPosition;

uniform float glossiness;
//uniform vec3 diffuseColor;
//uniform vec3 specularColor;

uniform vec3 cameraPosition;

layout (std140) uniform Lights{
	vec3 ambientLight;
	vec3 pointLight0Intensity;
	vec3 pointLight0pos;
};

uniform int brdfMode;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

uniform samplerCube skybox;

void main(){

	vec3 worldNml = normalize(worldNormal);
	vec3 viewDir = normalize(cameraPosition - worldPosition);
	vec3 reflectDir = reflect(-viewDir, worldNml);

	vec3 pointLight0Dir = normalize(pointLight0pos - worldPosition);
	vec3 pointLight0HalfDir = normalize(viewDir + pointLight0Dir);

	float viewTerm = dot(worldNml, viewDir);
	viewTerm = ceil(viewTerm);
	viewTerm = clamp(viewTerm, 0, 1);

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
	specular += specularTerm * pointLight0Intensity;

	vec3 ambient = ambientLight;
	
	if(brdfMode == 0){
		// sample diffuse texture color
		vec4 diffuseTextureColor = texture(diffuseTexture, texcoord);
		vec4 specularTextureColor = texture(specularTexture, texcoord);
		diffuse *= vec3(diffuseTextureColor);
		specular *= vec3(specularTextureColor);
		ambient *= vec3(diffuseTextureColor);
		daColor = vec4(viewTerm*(diffuse + specular) + ambient, 1.0);
	}else if(brdfMode == 1){
		vec3 reflecColor = texture(skybox, reflectDir).rgb;
		daColor = vec4(diffuse*.6 + specular + reflecColor*.5 + ambient*.5, 1.0);
	}
}