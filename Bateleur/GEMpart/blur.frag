uniform sampler2D tex0;
uniform vec2 distance;
uniform vec2 TX;
varying vec2 texcoord0;

void main (void) 
{ 

	vec2 texcoord = (gl_TextureMatrix[0] * gl_TexCoord[0]).st;
	vec4 sample = 0.5 * texture2D(tex0, texcoord - distance);
	sample += texture2D(tex0, texcoord);
	sample += 0.5 * texture2D(tex0, texcoord + distance);

	sample /= 2.;

	gl_FragColor =  sample;
} 


