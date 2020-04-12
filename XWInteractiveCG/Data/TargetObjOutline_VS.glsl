#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out VS_OUT {
    vec4 normal;
} vs_out;

uniform mat4 objectToWorld;
uniform mat3 objNmlToWorld;
uniform mat4 worldToView;

void main(){
	vec4 pos = vec4(aPos, 1.);
	pos = worldToView * objectToWorld * pos;

	vec4 nml = worldToView * vec4(objNmlToWorld * aNormal, .0);
	vs_out.normal = nml;

	gl_Position = pos;
}