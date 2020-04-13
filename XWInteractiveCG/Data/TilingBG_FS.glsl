#version 430 core

layout (location = 0) out vec4 daColor;

uniform sampler2D bgTex;

void main(){
	vec2 uv = gl_FragCoord.xy + vec2(.5, .5);
	uv = uv / 800.;
	uv = uv * 10.;

	uv = vec2(uv.x, 1 - uv.y);
	uv = uv + vec2(.2, -.2);
	vec3 col = texture(bgTex, uv).xyz;

	float tintWeight = gl_FragCoord.y + .5;
	tintWeight = tintWeight / 800.;

	vec3 topT = vec3(1);
	vec3 botT = vec3(.566, .726, .835);
	vec3 tint = mix(botT, topT, tintWeight);

	daColor = vec4(col * tint, 1.);
}