/*
 Copyright (c) 2010 David Petrie
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 */

#include "OpenGLInterface.h"
#include <string.h>

#define DIFONT_GLUE_MAX_VERTICES 32768
#define DIFONT_GLUE_MAX_MESHES 64

typedef struct
{
    GLfloat position[4];
    GLfloat color[4];
    GLfloat texCoord[2];
} difontVertex_t;


typedef struct {
    difontVertex_t vertices[DIFONT_GLUE_MAX_VERTICES];
    short quadIndices[DIFONT_GLUE_MAX_VERTICES * 3 / 2];
    difontVertex_t currVertex;
    unsigned int currIndex;
} difontGlueArrays_t;

difontGlueArrays_t difontGlueArrays;

GLenum difontCurrentPrimitive = GL_TRIANGLES;
bool DIFONTQuadIndicesInitted = false;


void difontBindPositionAttribute(GLint attributeHandle) {
    glVertexAttribPointer(attributeHandle, 4, GL_FLOAT, 0, sizeof(difontVertex_t), difontGlueArrays.vertices[0].position);
    glEnableVertexAttribArray(attributeHandle);
}


void difontBindColorAttribute(GLint attributeHandle) {
    glVertexAttribPointer(attributeHandle, 4, GL_FLOAT, 0, sizeof(difontVertex_t), difontGlueArrays.vertices[0].color);
    glEnableVertexAttribArray(attributeHandle);
}


void difontBindTextureAttribute(GLint attributeHandle) {
    glVertexAttribPointer(attributeHandle, 2, GL_FLOAT, 0, sizeof(difontVertex_t), difontGlueArrays.vertices[0].texCoord);
    glEnableVertexAttribArray(attributeHandle);
}


GLvoid difont::gl::Begin(GLenum prim) {
    if (!DIFONTQuadIndicesInitted) {
        for (int i = 0; i < DIFONT_GLUE_MAX_VERTICES * 3 / 2; i += 6) {
            int q = i / 6 * 4;
            difontGlueArrays.quadIndices[i + 0] = q + 0;
            difontGlueArrays.quadIndices[i + 1] = q + 1;
            difontGlueArrays.quadIndices[i + 2] = q + 2;

            difontGlueArrays.quadIndices[i + 3] = q + 0;
            difontGlueArrays.quadIndices[i + 4] = q + 2;
            difontGlueArrays.quadIndices[i + 5] = q + 3;
        }
        DIFONTQuadIndicesInitted = true;
    }

    difontGlueArrays.currIndex = 0;
    difontCurrentPrimitive = prim;
}


GLvoid difont::gl::Vertex3f(float x, float y, float z) {
    if (difontGlueArrays.currIndex >= DIFONT_GLUE_MAX_VERTICES) {
        return;
    }

    difontGlueArrays.currVertex.position[0] = x;
    difontGlueArrays.currVertex.position[1] = y;
    difontGlueArrays.currVertex.position[2] = z;
    difontGlueArrays.currVertex.position[3] = 1.0f;
    difontGlueArrays.vertices[difontGlueArrays.currIndex] = difontGlueArrays.currVertex;
    difontGlueArrays.currIndex++;
}


GLvoid difont::gl::Vertex2f(float x, float y) {
    if (difontGlueArrays.currIndex >= DIFONT_GLUE_MAX_VERTICES) {
        return;
    }

    difontGlueArrays.currVertex.position[0] = x;
    difontGlueArrays.currVertex.position[1] = y;
    difontGlueArrays.currVertex.position[2] = 0.0f;
    difontGlueArrays.currVertex.position[3] = 1.0f;
    difontGlueArrays.vertices[difontGlueArrays.currIndex] = difontGlueArrays.currVertex;
    difontGlueArrays.currIndex++;
}


GLvoid difont::gl::Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    difontGlueArrays.currVertex.color[0] = r;
    difontGlueArrays.currVertex.color[1] = g;
    difontGlueArrays.currVertex.color[2] = b;
    difontGlueArrays.currVertex.color[3] = a;
}


GLvoid difont::gl::TexCoord2f(GLfloat s, GLfloat t) {
    difontGlueArrays.currVertex.texCoord[0] = s;
    difontGlueArrays.currVertex.texCoord[1] = t;
}


GLvoid bindArrayBuffers() {}


GLvoid difont::gl::BindTexture(unsigned int textureId) {
    GLint activeTextureID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeTextureID);
    if ((unsigned int)activeTextureID != textureId) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
}


GLvoid difont::gl::End() {
    /*
    if (difontGlueArrays.currIndex == 0) {
        DIFONTCurrentPrimitive = 0;
        return;
    }

    if (DIFONTCurrentPrimitive == GL_QUADS) {
        glDrawElements(GL_TRIANGLES, difontGlueArrays.currIndex / 4 * 6, GL_UNSIGNED_SHORT, difontGlueArrays.quadIndices);
    } else {
        glDrawArrays(DIFONTCurrentPrimitive, 0, difontGlueArrays.currIndex);
    }*/
}


uint32_t difont::gl::VertexSize() {
    return sizeof(difontVertex_t);
}


uint32_t difont::gl::VertexCount() {
    return difontGlueArrays.currIndex;
}

/*
void difont::gl::CopyMesh(void *dataPointer, uint32_t *dataLen, uint32_t *vertexCount) {
    if (difontCurrentPrimitive == 0 || difontGlueArrays.currIndex == 0) {
        dataPointer = NULL;
        dataLen = 0;
        vertexCount = 0;
        return;
    }

    memcpy(dataPointer, difontGlueArrays.vertices, sizeof(difontVertex_t) * difontGlueArrays.currIndex);
    *vertexCount = difontGlueArrays.currIndex;
    *dataLen = sizeof(difontVertex_t) * difontGlueArrays.currIndex;
}*/


GLvoid difont::gl::Error(const char *source) {
	GLenum error = glGetError();
	 
	switch (error) {
		case GL_NO_ERROR:
			break;
		case GL_INVALID_ENUM:
			printf("GL Error (%x): GL_INVALID_ENUM. %s\n\n", error, source);
			break;
		case GL_INVALID_VALUE:
			printf("GL Error (%x): GL_INVALID_VALUE. %s\n\n", error, source);
			break;
		case GL_INVALID_OPERATION:
			printf("GL Error (%x): GL_INVALID_OPERATION. %s\n\n", error, source);
			break;/*
		case GL_STACK_OVERFLOW:
			printf("GL Error (%x): GL_STACK_OVERFLOW. %s\n\n", error, source);
			break;
		case GL_STACK_UNDERFLOW:
			printf("GL Error (%x): GL_STACK_UNDERFLOW. %s\n\n", error, source);
			break;*/
		case GL_OUT_OF_MEMORY:
			printf("GL Error (%x): GL_OUT_OF_MEMORY. %s\n\n", error, source);
			break;
		default:
			printf("GL Error (%x): %s\n\n", error, source);
			break;
	}
}
