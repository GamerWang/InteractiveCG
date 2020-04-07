#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec2 eTexcoord[];
in vec3 eWorldPosition[];
in vec4 eViewNormal[];

out vec2 texcoord;
out vec3 worldPosition;

uniform sampler2D dispMap;

void main(){
	for(int i = 0; i < 3; i++){
		texcoord = eTexcoord[i];
		float disp = texture(dispMap, vec2(texcoord.x, 1 - texcoord.y)).r;
		worldPosition = eWorldPosition[i];
		gl_Position = gl_in[i].gl_Position + eViewNormal[i] * 23 * disp;
		EmitVertex();
	}
	EndPrimitive();
}