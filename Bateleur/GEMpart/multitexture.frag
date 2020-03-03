#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D MyTex0;
uniform sampler2D MyTex1;
uniform sampler2D MyTex2;
uniform sampler2D MyTex3;
uniform sampler2D MyTex4;

varying vec2 texcoord0;
varying vec2 texcoord1;
varying vec2 texcoord2;
varying vec2 texcoord3;
varying vec2 texcoord4;

uniform vec2 offset;
uniform vec2 offsetmin;
uniform vec2 offsetmax;
uniform float time;
uniform float noise;
uniform float noisesize;
uniform float vsinamp;
uniform float vsinfreq;
uniform float vsinsquare;

uniform float vsinoffset;
uniform float vsinCamp;
uniform float delayMix;
//uniform float mixsize;
uniform float mixoffset;

//uniform bool doMask = true;
uniform bool showMask;

uniform float amountMask;
//uniform float amountDelayMask;

// blur -------------------------
/*#define BLUR_STEP4(n) color + = 0.5 * (texture2D(tex, texcoord + distance) + texture2D(tex, texcoord - distance))
vec4 blur(sampler2D tex, vec2 texcoord, vec2 distance) {
	vec4 color = texture2D(tex0, texcoord);
	color + = 0.5 * (texture2D(tex, texcoord + distance) + texture2D(tex, texcoord - distance));*/

// Brightness Contrast Saturation
uniform float brightness = 1.0, contrast = 1.0, saturation = 1.0;

vec4 BCS(vec4 col) {
	const vec3 lumCoeff = vec3(0.2125,  0.7154, 0.0721);

	col.rgb = clamp((col.rgb - vec3(step(brightness, 0.0)) ) * brightness, 0.0, 1.0);
	float l = min(1, 5 * length(col.rgb));

	//col.rgb /= col.a;

	col.rgb = (col.rgb - 0.5) * contrast + max((1.0 - contrast / 2.0), 0.5) * abs(brightness)/*0.5*/ ;

	vec3 intens = vec3(dot(col.rgb, lumCoeff));
	col.rgb = mix(intens, col.rgb, saturation);

	//col.rgb *= col.a;
	col.rgb *= pow(l,0.1);
	return col;
}

// MASK -------------------------
/*uniform float far = 0.1;
uniform float near = 0.9;
uniform float maskSmooth = 0.01;*/
vec4 Mask(vec4 col, float mask, float amount) {
	/*col.a *= smoothstep(far, far + maskSmooth, mask);
	col.a *= (1.0 - smoothstep(near, near + maskSmooth, mask));*/
	//col.a *= amount * mask + 1.0 - amount;
	col.a *= amount * mask + 1.0 - amount;
	//col *= amount * mask + 1.0 - amount;
	return col;
}

// KINECT CAMERA
uniform float showKinect = 0.0;
vec4 mixKinect(vec4 col, vec4 kinectCol) {
	return col * (1.0 - showKinect) + kinectCol * showKinect;
}
// SOLARIZATION -----------------
uniform float solar;
uniform float centerBrightness; // 0 - 1
uniform float powerCurve; // 0 - 4
uniform float colorize; // 0 - 1
uniform bool inverse; // BOOL

vec3 rgb2hsv(vec3 c)	{
	vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	//vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
	//vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
	vec4 p = c.g < c.b ? vec4(c.bg, K.wz) : vec4(c.gb, K.xy);
	vec4 q = c.r < p.x ? vec4(p.xyw, c.r) : vec4(c.r, p.yzx);
	
	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)	{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 solarize(vec4 inColor)
{
	vec4		hslColor;
	vec4		outColor;
	
	//	convert to HSV
	hslColor.rgb = rgb2hsv(inColor.rgb);
	outColor.rgb = hslColor.rgb;
	outColor.a = inColor.a;
	
	//	drop the saturation
	//outColor.g = 0.0;
	
	//	adjust the brightness curve
	outColor.b = pow(outColor.b, powerCurve);
	outColor.b = (outColor.b < centerBrightness) ? (1.0 - outColor.b / centerBrightness) : (outColor.b - centerBrightness) / centerBrightness;
	outColor.b = (inverse) ? 1.0 - outColor.b : outColor.b;
	
	outColor.g = (inverse) ? outColor.g * (1.0-hslColor.b) * colorize : outColor.g * hslColor.b * colorize;
	
	//	convert back to rgb
	outColor.rgb = hsv2rgb(outColor.rgb);
	
	return outColor;
}

vec3 solarize2(vec3 hsvColor)
{
	vec3 outColor;
	
	outColor.rgb = hsvColor.rgb;
	
	//	drop the saturation
	//outColor.g = 0.0;
	
	//	adjust the brightness curve
	outColor.b = pow(outColor.b, powerCurve);
	outColor.b = (outColor.b < centerBrightness) ? (1.0 - outColor.b / centerBrightness) : (outColor.b - centerBrightness) / centerBrightness;
	outColor.b = (inverse) ? 1.0 - outColor.b : outColor.b;
	
	outColor.g = (inverse) ? outColor.g * (1.0-hsvColor.b) * colorize : outColor.g * hsvColor.b * colorize;
	
	return outColor;
}

// POSTERIZATION -----------------
uniform float poster;
uniform float posterH;
uniform float posterS;
uniform float posterV;

vec3 posterize(vec3 hsvColor)
{
	vec3 outColor;
	
	outColor.r = hsvColor.r - mod(hsvColor.r, 1.0/(posterH + 1.0));	
	outColor.g = hsvColor.g - mod(hsvColor.g, 1.0/(posterS + 1.0));	
	outColor.b = hsvColor.b - mod(hsvColor.b, 1.0/(posterV + 1.0));	
	return outColor;
}

vec3 processHSV(vec3 inColor)
{
	vec3		hsvColor;
	vec3		outColor;
	
	//	convert to HSV
	hsvColor = rgb2hsv(inColor);

	hsvColor = mix(hsvColor, solarize2(hsvColor), solar);
	hsvColor = mix(hsvColor, posterize(hsvColor), poster);
	
	outColor = hsv2rgb(hsvColor);
	
	return outColor;
}

//----------------------------------------------------------------------------------------
//  1 out, 2 in...
float hash12(vec2 p, float proba)
{
	vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    //return smoothstep(noise*0.5, noise, fract((p3.x + p3.y) * p3.z));
    return step(fract((p3.x + p3.y) * p3.z), proba);
}

void main (void)
{
	float sinres = atan(sin(vsinfreq*texcoord0.y + vsinoffset)*(vsinsquare+1.0));
	//float mixval = mixprob;/*hash12(vec2(ivec2((texcoord0.y + mixoffset)*(mixsize+2.0))), mixprob);*/
	// float mixval = fract(ivec2(texcoord0.y*mixsize));
 
	vec4 color0 = texture2D(MyTex0, texcoord0 + vec2(vsinamp*sinres, 0));
	color0.rgb = color0.rgb*(sinres*vsinCamp + 1.0)+(1.0-color0.rgb)*(-sinres*vsinCamp);
	vec4 color1 = texture2D(MyTex1, texcoord1 + vec2(vsinamp*sinres, 0)/*clamp(texcoord1 + offset + vec2(vsinamp*sinres, 0), offsetmin, offsetmax)*/); // delayed
	//vec4 color1 = texture2D(MyTex1, texcoord1 + offset); 
	vec4 color2 = texture2D(MyTex2, texcoord2); // profondeur
	vec4 color3 = texture2D(MyTex3, texcoord3); // kinect
	vec4 color4 = texture2D(MyTex4, texcoord4); // delayed mask

	color0 = Mask(color0, color2.r, min(amountMask + delayMix, 1.0));
	color1 = Mask(color1, color4.r, amountMask);
	//gl_FragData[0] =
	// gl_FragColor = color0 * (color2.r) + color1 * (1.0 - color2.r) + hash12((vec2(ivec2(texcoord0*noisesize))*10000.0)/noisesize + time);
	color1.a *= delayMix;
	//color1 *= 1.0 - color0.a;
	//color0.rgb *= color0.a;
	gl_FragColor.rgb = mix(color1.rgb, color0.rgb, color0.a);
	gl_FragColor.a = max(color0.a, color1.a);
	gl_FragColor.a = color0.a + color1.a;
	//gl_FragColor = color0 * color0.a + color1 * (1.0 - color0.a);
	//color1.rgb *= color1.a;
//color1 = vec4(0.0);
	//color0 *= (1.0 - mixval);
	//color0 *= step(fract(color2.g * tigre ), 0.5);
	//color0 *= step(color2.g, tigre);
	//color0 *= smoothstep(tigre + 0.01, tigre, color2.r );
	//color0.rgb *= color0.a;
	//gl_FragColor = color1 + color0;
	
	gl_FragColor = BCS(gl_FragColor);
	gl_FragColor += hash12((vec2(ivec2(texcoord0*noisesize))*10000.0)/noisesize + time, noise);
//gl_FragColor = color2;
	//if(solar) gl_FragColor = solarize (gl_FragColor);
	gl_FragColor.rgb = processHSV(gl_FragColor.rgb);
	//if(doMask) 
	//gl_FragColor = Mask(gl_FragColor, color2.r, amountMask);
	//gl_FragColor = vec4(smoothstep(tigre , tigre - 0.1, color2.r ));
	//gl_FragColor = vec4(step(fract(color2.r * tigre), 0.5));
	if(showMask) gl_FragColor = color2;
	gl_FragColor = mixKinect(gl_FragColor, color3);
}

