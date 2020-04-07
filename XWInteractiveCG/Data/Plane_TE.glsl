#version 430 core

layout (quads, equal_spacing, ccw) in;

in vec2 cTexcoord[];
in vec3 cWorldPosition[];
in vec4 cViewNormal[];

out vec2 eTexcoord;
out vec3 eWorldPosition;
out vec4 eViewNormal;

float BCoord(int i, float u)
{
	const vec4 bc = vec4(1, 3, 3, 1);
	return bc[i] * pow(u, i) * pow(1.0 - u, 3 - i);
}

void main(void)
{
	vec4 pos = vec4(0);
	vec2 et = vec2(0);
	vec3 ew = vec3(0);
	vec4 ev = vec4(0);

	float u = gl_TessCoord.x, v = gl_TessCoord.y;

	vec4 bc = vec4(0);
	bc[0] = (1 - u) * (1 - v);
	bc[1] = v * (1 - u);
	bc[2] = v * u;
	bc[3] = u * (1 - v);

//	{
//		vec4 p0 = mix(gl_in[0].gl_Position, gl_in[3].gl_Position, u); // p0 = 0*(1-u) + 3*u
//		vec4 p1 = mix(gl_in[1].gl_Position, gl_in[2].gl_Position, u); // p1 = 1*(1-u) + 2*u
//		pos = mix(p0, p1, v); // pos = 0*(1-u)*(1-v) + 3*u*(1-v) + 1*(1-u)*v + 2*u*v
//	}
		
	for (int i = 0; i < 4; i++){
		pos += bc[i] * gl_in[i].gl_Position;
		et += bc[i] * cTexcoord[i];
		ew += bc[i] * cWorldPosition[i];
		ev += bc[i] * cViewNormal[i];
	}

	eTexcoord = et;

	eWorldPosition = ew;

	eViewNormal = ev;

	gl_Position = pos;
}
