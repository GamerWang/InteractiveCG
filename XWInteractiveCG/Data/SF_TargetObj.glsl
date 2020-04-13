#version 330 core

layout (location = 0) out vec4 daColor;

in VS_OUT{
	vec2 texcoord;
	vec3 worldPosition;
	vec3 worldNormal;
	vec3 worldTangent;
	vec3 worldBiTangent;
	vec4 viewNormal;
} fs_in;

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

struct SpecMod{
	float tu, tv;
	float su, sv;
	float psu, pou;
};

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

vec3 RotateAroundAxis(vec3 center, vec3 original, vec3 u, float angle){
	original -= center;
	float C = cos(angle);
	float S = sin(angle);


	return vec3(0);
}

void ModifyHalfDir(inout vec3 h, vec3 du, vec3 dv, vec3 nml, SpecMod sm){
	// translate
	h = h + sm.tu * du + sm.tv * dv;
	h = normalize(h);

	//  rotate

	// directional scaling
	h = h + sm.su * dot(h, du) * du + sm.sv * dot(h, dv)*dv;
	h = normalize(h);

	// split
	h = h + sm.psu * sign(dot(h + sm.pou * du, du)) * du;
	h = normalize(h);

}

float CalcSpecTerm(vec3 nml, vec3 hDir, float gls){
	float specTerm = dot(nml, hDir);
	specTerm = clamp(specTerm, 0, 1);
	specTerm = pow(specTerm, gls);
	return specTerm;
}

void main(){
	vec2 uv = fs_in.texcoord;
	vec3 wPos = fs_in.worldPosition;
	vec4 vNml = fs_in.viewNormal;
	vec3 wNml = normalize( fs_in.worldNormal);
	vec3 wTan = normalize( fs_in.worldTangent);
	vec3 wBTan = normalize( fs_in.worldBiTangent);
	vec3 vDir = normalize(cameraPosition - wPos);

	vec3 reflectDir = reflect(-vDir, wNml);
	vec2 matcapUV = normalize(vNml).xy * .5 + vec2(.5, .5);

	vec3 pl0Dir = normalize(pointLight0pos - wPos);

	// we can modify h to get stylized highlight effects
	vec3 pl0HDir = normalize(vDir + pl0Dir);

	vec3 pl0HDir1 = pl0HDir;
	SpecMod sm1 = SpecMod(.2,.20, -.6, .5, -.17, .1);

	vec3 pl0HDir2 = pl0HDir;
	SpecMod sm2 = SpecMod(-.45, .99, -.95, -.30, .0, .0);

	vec3 pl0HDir3 = pl0HDir;
	SpecMod sm3 = SpecMod(-.25,-.99, -.95, -.1, .0, .0);

	ModifyHalfDir(pl0HDir1, wTan, wBTan, wNml, sm1);
	ModifyHalfDir(pl0HDir2, wTan, wBTan, wNml, sm2);
	ModifyHalfDir(pl0HDir3, wTan, wBTan, wNml, sm3);

	float viewTerm = dot(wNml, vDir);
	float lightTerm = dot(wNml, pl0Dir);

	// cell yin
	float threshHold = -.3;
	float yin = step(threshHold, lightTerm);

	// cell shadow
	float shadow = ShadowCalculation(wPos);

	float finalDark = (1.-shadow) * yin;

	vec3 brightColor = vec3(1, 1, 1);
	vec3 darkColor = vec3(.6, .3, .3);

	vec3 finalTint = mix(darkColor, brightColor, yin);

	// handling pointLight0
	// diffuse part
	vec3 diffuse = vec3(0);

	// specular part
	vec3 specular = vec3(0);
	float specularTerm = 0;
	float specTerm1 = CalcSpecTerm(wNml, pl0HDir1, glossiness);
	float specTerm2 = CalcSpecTerm(wNml, pl0HDir2, glossiness);
	float specTerm3 = CalcSpecTerm(wNml, pl0HDir3, glossiness);

	specularTerm = specTerm1 + specTerm2 + specTerm3;

	float specLimit = .4f;
	if(specularTerm > 1 - specLimit){
		specularTerm = 1.f;
	}else{
		specularTerm = 0.f;
	}

	specular += specularTerm * pointLight0Intensity;

	vec3 ambient = ambientLight;

	//diffuse *= (1.-shadow);
	//specular *= (1.-shadow);
	//specular *= finalDark;
	specular *= yin;

	// sample diffuse texture color
	vec4 diffuseTextureColor = texture(diffuseTexture, uv);
	float matcapMask = texture(matcapMask, uv).r;
	vec4 matcapColor = texture(matcapTexture, matcapUV);
	matcapColor = mix(vec4(1), matcapColor * 1.8, matcapMask*.5);

	// vec4 specularTextureColor = texture(specularTexture, uv);
	vec4 specularTextureColor = vec4(1);

	diffuse = vec3(diffuseTextureColor * matcapColor);
	specular *= vec3(specularTextureColor * matcapMask);

	daColor = vec4((finalTint * diffuse) + specular, 1.);
	//daColor = vec4(wNml/2. + .5, 1.);
	//daColor = vec4(wTan/2. + .5, 1.);
	//daColor = vec4(wBTan/2. + .5, 1.);
}