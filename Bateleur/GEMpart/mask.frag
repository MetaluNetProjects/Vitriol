#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D MyTex0;
uniform sampler2D MyTex1;

varying vec2 texcoord0;
varying vec2 texcoord1;

uniform float amountMask;


void main (void)
{
	gl_FragColor = texture2D(MyTex0, texcoord0);
	vec4 mask = texture2D(MyTex1, texcoord1);

	gl_FragColor.a = mask.r;
}

