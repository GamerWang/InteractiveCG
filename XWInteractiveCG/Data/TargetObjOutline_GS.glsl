#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec4 normal;
} gs_in[];

uniform mat4 viewToClip;

void main()
{
	float outlineWidth = .1f;
	float zOffset = .005f;

	for(int i = 0; i < 3; i++){
		vec4 p = gl_in[i].gl_Position;
		p = p + gs_in[i].normal * outlineWidth;
		p = viewToClip * p;
		p = p + vec4(.0, .0, zOffset, .0);
		gl_Position = p;
		EmitVertex();
	}
	EndPrimitive();
}