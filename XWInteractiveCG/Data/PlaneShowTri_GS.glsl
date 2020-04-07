#version 430 core
layout (triangles) in;
layout (line_strip, max_vertices = 4) out;

in vec2 eTexcoord[];
in vec4 eViewNormal[];

uniform sampler2D dispMap;

void main() {
	for(int i = 0; i < 3; i++){
		vec2 texcoord = eTexcoord[i];
		float disp = texture(dispMap, vec2(texcoord.x, 1 - texcoord.y)).r;
		gl_Position = gl_in[i].gl_Position + eViewNormal[i] * 23 * disp - vec4(0, 0, .001, 0);
		EmitVertex();
	}
	vec2 texcoord = eTexcoord[0];
	float disp = texture(dispMap, vec2(texcoord.x, 1 - texcoord.y)).r;
	gl_Position = gl_in[0].gl_Position + eViewNormal[0] * 23 * disp - vec4(0, 0, .001, 0);
	EmitVertex();
	EndPrimitive();
}
