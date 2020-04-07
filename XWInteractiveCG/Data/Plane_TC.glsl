#version 430 core

layout(vertices = 4) out;

in vec2 texcoord[];
in vec3 worldPosition[];
in vec4 viewNormal[];


out vec2 cTexcoord[];
out vec3 cWorldPosition[];
out vec4 cViewNormal[];
const int AB = 2;
const int BC = 3;
const int CD = 0;
const int DA = 1;

uniform int tessellationFactor;
uniform float tessellationSlope;
uniform float tessellationShift;
uniform vec3 cameraPosition;

float LodFactor(float dist) {
	float tessellationLevel = max(0.0, tessellationFactor/pow(dist, tessellationSlope) + tessellationShift);
	return tessellationLevel;
}

void main() {
	if(gl_InvocationID == 0) {
		gl_TessLevelOuter[0] = 64.;
		gl_TessLevelOuter[1] = 64.;
		gl_TessLevelOuter[2] = 64.;
		gl_TessLevelOuter[3] = 64.;

		gl_TessLevelInner[0] = 64.;
		gl_TessLevelInner[1] = 64.;
	}

	cTexcoord[gl_InvocationID] = texcoord[gl_InvocationID];
	cWorldPosition[gl_InvocationID] = worldPosition[gl_InvocationID];
	cViewNormal[gl_InvocationID] = viewNormal[gl_InvocationID];

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}