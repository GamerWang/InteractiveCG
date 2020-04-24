#version 430 core
layout (location = 0) out vec4 daColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec2 texelSize;

void main()
{
	vec2 offsets[9] = vec2[](
        vec2(-1,  1), // top-left
        vec2( 0.0f,    1), // top-center
        vec2( 1,  1), // top-right
        vec2(-1,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( 1,  0.0f),   // center-right
        vec2(-1, -1), // bottom-left
        vec2( 0.0f,   -1), // bottom-center
        vec2( 1, -1)  // bottom-right    
    );

	float edgeKernel[9] = float[](
		1., 1., 1.,
		1., -8., 1.,
		1., 1., 1.
	);

	vec3 sampleTex[9];
	for(int i = 0; i < 9; i++){
		sampleTex[i] = texture(screenTexture, TexCoords + offsets[i] * texelSize).rgb;
	}
	vec3 ppCol = vec3(.0);
	for(int i = 0; i < 9; i++){
		ppCol += sampleTex[i] * edgeKernel[i];
	}

	float ppWeight = ppCol.r + ppCol.g + ppCol.b;
	ppWeight = clamp(ppWeight, .0, 1.);

    vec3 col = texture(screenTexture, TexCoords).rgb;
	vec3 lineCol = vec3(.2, .2, .2);
	vec3 finalCol = mix(col, lineCol, ppWeight);
    daColor = vec4(col, 1.0);
	daColor = vec4(finalCol, 1.);
} 