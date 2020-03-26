#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 4) out;

void main() {
	for(int i = 0; i < 3; i++){
		gl_Position = gl_in[i].gl_Position + vec4(0, 0, .01, 0);
		EmitVertex();
	}
	gl_Position = gl_in[0].gl_Position + vec4(0, 0, .01, 0);
	EmitVertex();
	EndPrimitive();
}
