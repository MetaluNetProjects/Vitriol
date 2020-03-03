//uniform sampler2D MyTex0;

//varying vec2 texcoord0;
const float W = 40.0;
const float H = 30.0;
const float res = 10.0;

void main (void)
{
	vec2 texcoord0 = (gl_TextureMatrix[0] * gl_TexCoord[0]).st;
	float d1 = distance(texcoord0, vec2(1.0 * W, -1.0 * H));
	float d2 = distance(texcoord0, vec2(-1.0 * W, -1.0 * H));
	float d3 = distance(texcoord0, vec2(-1.0 * W, 1.0 * H));
	float d4 = distance(texcoord0, vec2(1.0 * W, 1.0 * H));
	
	float x = (d1 - d2 + d4 - d3) * 0.5;
	float y = (d4 - d1 + d3 - d2) * 0.5;
	
	gl_FragColor = vec4(vec3(mod(floor(x * res) + floor(y * res), 2.0)), 1.0);
}

