//
#version 120
#extension GL_ARB_texture_rectangle : enable

void main() {
	//gl_Position = gl_Vertex;
	gl_FrontColor = gl_Color;

	gl_TexCoord[0] = gl_MultiTexCoord0;
	// perform standard transform on vertex
	gl_Position = ftransform();

}

