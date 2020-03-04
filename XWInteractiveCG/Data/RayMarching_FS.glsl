#version 330 core

layout (location = 0) out vec4 daColor;

in vec2 texcoord;

uniform float time;

vec2 sincos( float x) { return vec2(sin(x), cos(x)); }

vec2 sdCylinder(in vec3 p){
	return vec2(length(p.xz), (p.y+50)/100);
}

vec3 opU ( vec3 d1, vec3 d2) { return (d1.x < d2.x)? d1:d2; }

vec3 map( vec3 p) {
	vec2 id = floor((p.xz+1)/2);
	p.xz = mod(p.xz+1, 2) - 1;
	
	float ph = sin(.5 + 3.1*id.x + sin(7.1*id.y));

	p.xz += 0.5*sincos(1+.5*time+(p.y+11*ph)*.8);
	
	vec3 p1 = p; p1.xz += .25*sincos(1*p.y-1*time+0);
	vec3 p2 = p; p2.xz += .25*sincos(1*p.y-0*time+2);
	vec3 p3 = p; p3.xz += .25*sincos(1*p.y-0*time+4);

	vec2 h1 = sdCylinder(p1);
	vec2 h2 = sdCylinder(p2);
	vec2 h3 = sdCylinder(p3);

	return opU(opU(
		vec3(h1.x-.12-.01*cos(cos(2000*h2.y-time*10)), ph+0/3, h1.y),
		vec3(h2.x-.12-.01*cos(2000*h2.y-time*0), ph + 1/3, h2.y)),
		vec3(h3.x-.12-.01*cos(2000*h3.y-time*0), ph + 2/3, h3.y));
}

vec3 calcNormal(in vec3 pos, in float dt)
{
	vec2 e = vec2(1, -1)*dt;
	return normalize( 
		e.xyy*map(pos+e.xyy).x +
		e.yyx*map(pos+e.yyx).x +
		e.yxy*map(pos+e.yxy).x +
		e.xxx*map(pos+e.xxx).x);
}

float calcOcc(in vec3 pos, in vec3 nor)
{
	const float h = .15;
	float ao = 0;
	for(int i = 0; i < 8; i++)
	{
		vec3 dir = sin(float(i)*vec3(1,7.13,13.71)+vec3(0,2,4));
		dir = dir+2.5*nor*max(0, -dot(nor,dir));
		float d = map(pos + h*dir).x;
		ao += max(0, h-d);
	}
	return clamp(1.0 - 7.0*ao, 0, 1);
}

vec3 shade(in float t, in float m, in float v, in vec3 ro, in vec3 rd){
	float px = 0.0001;
	float eps = px*t;

	vec3 pos = ro + t*rd;
	vec3 nor = calcNormal(pos, eps);
	float occ = calcOcc(pos, nor);

	vec3 col = .5 + .5*cos(m*vec3(1.4, 1.2, 1.0) + vec3(0, 1, 2));
	col += .05 * nor;
	col = clamp(col, 0, 1);
	col *= 1 + .5*nor.x;
	col += .2*clamp(1+dot(rd, nor), 0, 1);
	col *= 1.4;
	col *= occ;
	col *= exp(-0.15*t);
	col *= 1 - smoothstep(15, 35, t);

	return col;
}

vec2 toFragCoord(vec2 texcoord){
	vec2 iResolution = vec2(800, 800);
	vec2 fragCoord = texcoord.xy * vec2(1, -1) + vec2(0, 1);
	fragCoord *= iResolution;
	return fragCoord;
}

void main(){
	vec2 iResolution = vec2(800, 800);
	vec2 fragCoord = toFragCoord(texcoord);
	vec2 p = (-iResolution.xy + 2.0 * fragCoord.xy) / iResolution.y;

	vec3 ro = 0.6*vec3(2.0, -3.0, 4.0);
	vec3 ta = 0.5*vec3(0.0, 4.0, -4.0);

	float fl = 1.0;
	vec3 ww = normalize(ta - ro);
	vec3 uu = normalize(cross(vec3(1, 0, 0), ww));
	vec3 vv = normalize(cross(ww, uu));
	vec3 rd = normalize(p.x*uu + p.y*vv + fl*ww);

	float px = (2.0 / iResolution.y)*(1.0/fl);

	vec3 col = vec3(0);

	//---------------------------------------------------
	// raymarch loop
	//---------------------------------------------------
	const float maxdist = 32.0;

	vec3 res = vec3(-1.0);
	float t = 0.0;
	
	// antialiasing
	vec3 oh = vec3(0);
	mat4 hit = mat4(
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1,
		-1, -1, -1, -1
		);
	vec4 tmp = vec4(0);

	for (int i=0; i < 128; i++) {
		vec3 h = map(ro+t*rd);
		float th1 = px*t;
		res = vec3(t, h.yz);
		if(h.x<th1 || t>maxdist) break;

		// antialiasing

		t+= min(h.x, .5)*.5;
	}

	if(t < maxdist)
		col = shade(res.x, res.y, res.z, ro, rd);

	col = pow(col, vec3(.5, .7, .5));
	vec2 q = fragCoord.xy/iResolution.xy;
	col *= pow(16*q.x*q.y*(1-q.x)*(1-q.y), 0.1);

	daColor = vec4(col, 1);
}