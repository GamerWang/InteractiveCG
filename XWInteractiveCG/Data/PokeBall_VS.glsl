#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texturecoord;

uniform mat4 objectToWorld;
uniform mat3 objNmlToWorld;
uniform mat4 worldToView;
uniform mat4 viewToClip;

out VS_OUT{
	vec2 texcoord;
	vec3 worldPosition;
	vec3 worldNormal;
	//vec3 worldTangent;
	//vec3 worldBiTangent;
	//vec4 viewNormal;
} vs_out;

void main(){
	vec4 p = vec4(pos, 1.);

	p = objectToWorld * p;
	vs_out.worldPosition = p.xyz;

	vs_out.worldNormal = objNmlToWorld * normal;

	p = viewToClip * worldToView * p;
	gl_Position = p;
}