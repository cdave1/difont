/*
 * Copyright (c) 2015 David Petrie david@davidpetrie.com
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software. Permission is granted to anyone to use this software for
 * any purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim
 * that you wrote the original software. If you use this software in a product, an
 * acknowledgment in the product documentation would be appreciated but is not
 * required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "HelloWorld.h"

#include "CommonMath.h"
#include "OpenGLShim.h"
#include <iostream>
#include <sstream>

//#define HELLO_WORLD_FONT_SIZE uint32_t(310.0f)
#define HELLO_WORLD_FONT_SIZE uint32_t(m_height * 0.251f)

static GLuint vertexArrayObject = 0;
static GLuint vertexBufferObject = 0;
static GLuint indexBufferObject = 0;
static unsigned int vertexCount = 0;
static unsigned int triangleCount = 0;

#define DBOUT( s )            \
{                             \
    std::ostringstream os_;    \
    os_ << s;                   \
    OutputDebugStringA( os_.str().c_str() );  \
}

typedef struct Triangle {
    uint32_t indexes[3];

    Triangle(uint32_t i1, uint32_t i2, uint32_t i3) {
        indexes[0] = i1;
        indexes[1] = i2;
        indexes[2] = i3;
    }
} Triangle;

void HelloWorld::SetupFonts(const char *fontpath) {
    glEnable(GL_TEXTURE_2D);
    m_font = new difont::PolygonFont(fontpath);
    if (m_font->Error()) {
        fprintf(stderr, "Could not load font at `%s'\n", fontpath);
        delete m_font;
        m_font = NULL;
    } else {
        m_font->FaceSize(HELLO_WORLD_FONT_SIZE);
        m_font->CharMap(FT_ENCODING_ADOBE_LATIN_1);
    }

    m_textureFont = new difont::TextureFont(fontpath);
    if (m_textureFont->Error()) {
        fprintf(stderr, "Could not load font at `%s'\n", fontpath);
        delete m_textureFont;
        m_textureFont = NULL;
    } else {
        m_textureFont->FaceSize(HELLO_WORLD_FONT_SIZE);
        m_textureFont->CharMap(FT_ENCODING_ADOBE_LATIN_1);
    }
}


void HelloWorld::SetupVertexArrays(GLuint shaderProgram) {
    if (m_font) {
        difont::FontMeshSet::Begin();
        m_font->Render("hello world!");

        vertexCount = 0;
        triangleCount = 0;

        unsigned int fontMeshCount = difont::FontMeshSet::MeshCount();
        difont::FontMesh *meshes = difont::FontMeshSet::GetMeshes();

        for (int i = 0; i < fontMeshCount; ++i) {
            difont::FontMesh fontMesh = meshes[i];
            vertexCount += fontMesh.GetVertexCount();

            if (fontMesh.primitive == GL_TRIANGLES) {
                triangleCount += fontMesh.GetVertexCount() / 3;
            }
            else if (fontMesh.primitive == GL_TRIANGLE_STRIP) {
                triangleCount += fontMesh.GetVertexCount() - 2;
            }
            else if (fontMesh.primitive == GL_TRIANGLE_FAN) {
                triangleCount += fontMesh.GetVertexCount() - 2;
            }
            else if (fontMesh.primitive == GL_QUADS) {
                triangleCount += 2 * (fontMesh.GetVertexCount() / 4);
            }
        }

        size_t vertexBlockSz = sizeof(difont::FontVertex) * vertexCount;
        difont::FontVertex *vertexBufferData = (difont::FontVertex *)malloc(vertexBlockSz);
        Triangle *indexBufferData = (Triangle *)malloc(sizeof(Triangle) * triangleCount);

        int vertexes = 0;
        int triangles = 0;

        for (int i = 0; i < fontMeshCount; ++i) {
            difont::FontMesh fontMesh = meshes[i];
            uint32_t vertexCount = fontMesh.GetVertexCount();

            for (int j = 0; j < vertexCount; ++j) {
                difont::FontVertex vertex = fontMesh.vertices[j];
                uint32_t index = vertexes + j;
                vertexBufferData[index] = vertex;
            }

            if (fontMesh.primitive == GL_TRIANGLES) {
                for (int j = 0; j < vertexCount - 2; j += 3) {
                    int i1 = vertexes + j;
                    int i2 = vertexes + (j + 1);
                    int i3 = vertexes + (j + 2);

                    indexBufferData[triangles++] = Triangle(i1, i2, i3);
                }
            }
            else if (fontMesh.primitive == GL_TRIANGLE_STRIP) {
                for (int j = 0; j < (vertexCount - 2); j++) {
                    int i1 = vertexes + j;
                    int i2 = vertexes + (j + 1);
                    int i3 = vertexes + (j + 2);

                    indexBufferData[triangles++] = Triangle(i1, i2, i3);
                }
            }
            else if (fontMesh.primitive == GL_TRIANGLE_FAN) {
                for (int j = 0; j < (vertexCount - 2); j++) {
                    int i1 = vertexes;
                    int i2 = vertexes + (j + 1);
                    int i3 = vertexes + (j + 2);

                    indexBufferData[triangles++] = Triangle(i1, i2, i3);
                }
            }
            else if (fontMesh.primitive == GL_QUADS) {
                for (int j = 0; j < vertexCount - 3; j += 4) {
                    int i1 = vertexes + j;
                    int i2 = vertexes + (j + 1);
                    int i3 = vertexes + (j + 2);
                    int i4 = vertexes + (j + 3);

                    indexBufferData[triangles++] = Triangle(i1, i2, i3);
                    indexBufferData[triangles++] = Triangle(i1, i3, i4);
                }
            }
            vertexes += vertexCount;
        }
        assert(vertexes == vertexCount);
        assert(triangles == triangleCount);

        if (indexBufferObject == 0)
            glGenBuffers(1, &indexBufferObject);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Triangle) * triangleCount, indexBufferData, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        free(indexBufferData);

        if (vertexBufferObject == 0)
            glGenBuffers(1, &vertexBufferObject);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, sizeof(difont::FontVertex) * vertexCount, vertexBufferData, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        free(vertexBufferData);

        if (vertexArrayObject == 0)
            glGenVertexArrays(1, &vertexArrayObject);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindVertexArray(vertexArrayObject);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        glVertexAttribPointer(glGetAttribLocation(shaderProgram, "Position"), 3, GL_FLOAT, GL_FALSE, sizeof(difont::FontVertex), DIFONT_BUFFER_OFFSET(0));
        glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "Position"));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        difont::FontMeshSet::End();
    }
}


void HelloWorld::Update(GLuint shaderProgram) {
    
}


void HelloWorld::Render(GLuint shaderProgram) {
    GLfloat width = 1800.0f;// GLfloat(viewportWidth());
    GLfloat height = 1200.0f; // GLfloat(viewportHeight());

    glUseProgram(shaderProgram);

    float projection[16];
    float view[16];
    float world[16];
    difont::examples::Math::MatrixIdentity(view);
    difont::examples::Math::MatrixIdentity(world);
    difont::examples::Math::Ortho(projection, -width, width, -height, height, -1.0f, 1.0f);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "ProjectionMatrix"), 1, GL_FALSE, projection);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "ViewMatrix"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "ModelMatrix"), 1, GL_FALSE, world);

    glBindVertexArray(vertexArrayObject);
    glDrawRangeElements(GL_TRIANGLES, 0, vertexCount - 1, triangleCount * 3, GL_UNSIGNED_INT, NULL);

    const unsigned MAX_COLORS = 4;
    color4_t colors[MAX_COLORS];
    vec4Set(colors[0], 0.8f, 0.1f, 0.0f, 1.0f);
    vec4Set(colors[1], 1.0f, 1.0f, 1.0f, 1.0f);
    vec4Set(colors[2], 0.0f, 0.1f, 0.8f, 1.0f);
    vec4Set(colors[3], 0.0f, 0.8f, 0.1f, 1.0f);

    for (unsigned i = 0; i < MAX_COLORS; ++i) {
        color4_t color;
        vec4Set(color, colors[i][0], colors[i][1], colors[i][2], colors[i][3]);
    }
}
