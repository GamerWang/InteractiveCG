#version 330 core

layout (location = 0) out vec4 daColor;

in vec2 texcoord;
in vec3 worldNormal;
in vec3 worldPosition;

uniform float glossiness;
uniform vec3 diffuseColor;
uniform vec3 specularColor;

uniform vec3 cameraPosition;

uniform vec3 ambientLight;
uniform vec3 pointLight0Intensity;
uniform vec3 pointLight0pos;

uniform sampler2D diffuseTexture;

void main(){
	vec3 worldNml = normalize(worldNormal);
	vec3 viewDir = normalize(cameraPosition - worldPosition);

	vec3 pointLight0Dir = normalize(pointLight0pos - worldPosition);
	vec3 pointLight0HalfDir = normalize(viewDir + pointLight0Dir);

	float viewTerm = dot(worldNml, viewDir);
	viewTerm = ceil(viewTerm);
	viewTerm = clamp(viewTerm, 0, 1);

	// sample diffuse texture color
	vec4 diffuseTextureColor = texture(diffuseTexture, texcoord);

	// handling pointLight0
	// diffuse part
	vec3 diffuse = vec3(0);
	float pointLight0geoTerm = dot(worldNml, pointLight0Dir);
	pointLight0geoTerm = clamp(pointLight0geoTerm, 0, 1);
	diffuse += pointLight0geoTerm * pointLight0Intensity;
	diffuse *= vec3(diffuseTextureColor);


	// specular part
	vec3 specular = vec3(0);
	float specularTerm = dot(worldNml, pointLight0HalfDir);
	specularTerm = clamp(specularTerm, 0, 1);
	specularTerm = pow(specularTerm, glossiness);
	specular += specularTerm * pointLight0Intensity;
	specular *= specularColor;

	vec3 ambient = ambientLight * vec3(diffuseTextureColor);

	daColor = vec4(viewTerm*(diffuse + specular) + ambient, 1.0);
}