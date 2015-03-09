#include "OpenGLShim.h"
#include "CommonMath.h"

static vertex_t vertices[MAX_VERTEX_COUNT];
static vertex_t vertex;
static GLenum currentPrimitive = GL_TRIANGLES;
static int vertexCount = 0;


void difont::examples::OpenGL::Begin(GLenum prim) {
	currentPrimitive = prim;
	vertexCount = 0;

	glVertexAttribPointer(RENDER_ATTRIB_VERTEX, 3, GL_FLOAT, 0, sizeof(vertex_t), vertices[0].xyz);
	glVertexAttribPointer(RENDER_ATTRIB_COLOR, 4, GL_FLOAT, 0, sizeof(vertex_t), vertices[0].rgba);
	
	glEnableVertexAttribArray(RENDER_ATTRIB_VERTEX);
	glEnableVertexAttribArray(RENDER_ATTRIB_COLOR);
}


void difont::examples::OpenGL::Vertex3f(float x, float y, float z) {
	if (vertexCount > MAX_VERTEX_COUNT) return;
    vec3Set(vertex.xyz, x, y, z);
	vertices[vertexCount] = vertex;
	vertexCount++;
}


void difont::examples::OpenGL::Color4f(float r, float g, float b, float a) {
    vec4Set(vertex.rgba, r, g, b, a);
}


void difont::examples::OpenGL::TexCoord2f(float s, float t) {
    vec2Set(vertex.st, s, t);
}


void difont::examples::OpenGL::End() {
	if (vertexCount == 0) {
		currentPrimitive = 0;
		return;
	}
	glDrawArrays(currentPrimitive, 0, vertexCount);
	vertexCount = 0;
	currentPrimitive = 0;
}
