#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texturecoord;
//layout (location = 3) in vec3 tangent;
//layout (location = 4) in vec3 biTangent;

out VS_OUT{
	vec2 texcoord;
	vec3 worldPosition;
	vec3 worldNormal;
	vec3 worldTangent;
	vec3 worldBiTangent;
	vec4 viewNormal;
} vs_out;

layout (std140) uniform Matrices{
	mat4 worldToClamp;
	mat4 worldToView;
	mat4 viewToProj;
};

uniform mat4 objectToWorldMatrix;
uniform mat3 objectNormalToWorldMatrix;

// standard clip plane value (0, 1, 0, 8) which clips everything below
uniform vec4 clipPlane;

uniform vec3 pointLight0pos;

void main(){
	vs_out.texcoord = vec2(texturecoord);

	vs_out.worldNormal = objectNormalToWorldMatrix * normal;

	// use assimp tangent Space
	//vs_out.worldTangent = objectNormalToWorldMatrix * tangent;
	//vs_out.worldBiTangent = objectNormalToWorldMatrix * biTangent;
	
	// calculate tangent manually
	{
		vec3 nml = normalize(normal);
		vec3 objFwd = vec3(0, 1, 0);
		vec3 biTan = cross(nml, objFwd);
		vec3 tan = cross(biTan, nml);

		vs_out.worldTangent = objectNormalToWorldMatrix * tan;
		vs_out.worldBiTangent = objectNormalToWorldMatrix * biTan;
	}
	
	vs_out.viewNormal = worldToView * vec4(vs_out.worldNormal, .0);

	vec4 v = vec4(pos, 1.0);
	v = objectToWorldMatrix * v;

	vs_out.worldPosition = vec3(v);

	// clip one side of the mirror when render to buffer
	gl_ClipDistance[0] = dot(vec4(vs_out.worldPosition, 1), clipPlane);

	v = worldToClamp * v;
	gl_Position = v;
}
