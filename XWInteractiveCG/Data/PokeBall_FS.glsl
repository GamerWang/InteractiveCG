#version 430 core

layout (location = 0) out vec4 daColor;

uniform vec3 color;
uniform vec3 yinClr;
uniform vec3 cameraPosition;

layout (std140) uniform Lights{
	vec3 ambientLight;
	vec3 pointLight0Intensity;
	vec3 pointLight0pos;
};

in VS_OUT{
	vec2 texcoord;
	vec3 worldPosition;
	vec3 worldNormal;
	//vec3 worldTangent;
	//vec3 worldBiTangent;
	//vec4 viewNormal;
} fs_in;

void main(){
	vec3 wPos = fs_in.worldPosition;
	vec3 wNml = normalize( fs_in.worldNormal);
	vec3 vDir = normalize(cameraPosition - wPos);
	vec3 reflectDir = reflect(-vDir, wNml);
	vec3 pl0Dir = normalize(pointLight0pos - wPos);

	float viewTerm = dot(wNml, vDir);
	float lightTerm = dot(wNml, pl0Dir);
	
	// cell yin
	float threshHold = -.3;
	float yin = step(threshHold, lightTerm);

	vec3 brightColor = vec3(1, 1, 1);
	vec3 darkColor = yinClr;
	vec3 finalTint = mix(darkColor, brightColor, yin);

	vec3 diffuse = vec3(0);
	diffuse = color * finalTint;

	daColor = vec4(diffuse, 1.);
}