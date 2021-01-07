//
#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect SourceTexture;
uniform float Scale;
void main(){
	vec4 velocity = texture2DRect(SourceTexture, gl_TexCoord[0].st);
	velocity.xyz *= vec3(Scale);
	velocity.w = pow(length(velocity.xyz), 0.33);
	velocity.xyz += vec3(0.5);
	gl_FragColor = velocity;
}

