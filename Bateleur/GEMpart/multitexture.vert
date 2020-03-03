#version 120
varying vec2 texcoord0;
varying vec2 texcoord1;
varying vec2 texcoord2;
varying vec2 texcoord3;
varying vec2 texcoord4;
/*uniform vec2 tex2scale = vec2(1.0,1.0);
uniform vec2 tex2offset;*/

void main()
{
	
    texcoord0 = (gl_TextureMatrix[0] * gl_MultiTexCoord0).st;
    texcoord1 = (gl_TextureMatrix[1] * gl_MultiTexCoord1).st;
    texcoord2 = (gl_TextureMatrix[2] * gl_MultiTexCoord2).st /** tex2scale + tex2offset*/;
    texcoord3 = (gl_TextureMatrix[3] * gl_MultiTexCoord3).st;
    texcoord4 = (gl_TextureMatrix[4] * gl_MultiTexCoord4).st;
    gl_Position = ftransform();
}
