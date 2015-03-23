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

static GLuint vertexArrayObject = 0;
static GLuint vertexBufferObject = 0;
static GLuint indexBufferObject = 0;
static unsigned int totalVertexes = 0;
static unsigned int totalIndexes = 0;
#define DBOUT( s )            \
{                             \
    std::ostringstream os_;    \
    os_ << s;                   \
    OutputDebugStringA( os_.str().c_str() );  \
}
void HelloWorld::Update(GLuint shaderProgram) {
    if (m_font) {
        difont::FontMeshSet::Begin();
        m_font->Render("hello world!");

        totalIndexes = 0;
        totalVertexes = 0;

        unsigned int fontMeshCount = difont::FontMeshSet::MeshCount();
        difont::FontMesh *meshes = difont::FontMeshSet::GetMeshes();

        for (int i = 0; i < fontMeshCount; ++i) {
            difont::FontMesh fontMesh = meshes[i];
            totalVertexes += fontMesh.GetVertexCount();
        }

        size_t vertexBlockSz = sizeof(difont::FontVertex) * totalVertexes;
        difont::FontVertex *vertexBufferData = (difont::FontVertex *)malloc(vertexBlockSz);
        uint32_t *indexBufferData = (uint32_t *)malloc(sizeof(uint32_t) * totalVertexes);

        int indexes = 0;
        for (int i = 0; i < fontMeshCount; ++i) {
            difont::FontMesh fontMesh = meshes[i];

            for (int j = 0; j < fontMesh.GetVertexCount(); ++j) {
                difont::FontVertex vertex = fontMesh.vertices[j];
                uint32_t index = totalIndexes + j;
                vertexBufferData[index] = vertex;
            }

            if (fontMesh.primitive == GL_TRIANGLES) {
                for (int j = 0; j < totalIndexes - 2; j += 3) {
                    int i1 = totalIndexes + j;
                    int i2 = totalIndexes + (j + 1);
                    int i3 = totalIndexes + (j + 2);

                    indexBufferData[i1] = i1;
                    indexBufferData[i2] = i2;
                    indexBufferData[i3] = i3;
                }
            }
            else if (fontMesh.primitive == GL_TRIANGLE_STRIP) {
                for (int j = 0; j < (totalIndexes - 2); j++) {
                    int i1 = totalIndexes + j;
                    int i2 = totalIndexes + (j + 1);
                    int i3 = totalIndexes + (j + 2);

                    indexBufferData[i1] = i1;
                    indexBufferData[i2] = i2;
                    indexBufferData[i3] = i3;
                }
            }
            else if (fontMesh.primitive == GL_TRIANGLE_FAN) {
                for (int j = 0; j < (totalIndexes - 2); j++) {
                    int i1 = totalIndexes;
                    int i2 = totalIndexes + (j + 1);
                    int i3 = totalIndexes + (j + 2);

                    indexBufferData[i1] = i1;
                    indexBufferData[i2] = i2;
                    indexBufferData[i3] = i3;
                }
            }
            else if (fontMesh.primitive == GL_QUADS) {
                for (int j = 0; j < vertexCount - 3; j += 4) {
                    int i1 = totalIndexes + j;
                    int i2 = totalIndexes + (j + 1);
                    int i3 = totalIndexes + (j + 2);
                    int i4 = totalIndexes + (j + 3);

                    Size4 quad(i1, i2, i3, i4);
                    mesh->AddQuad(quad);

                    indexBufferData[i1] = i1;
                    indexBufferData[i2] = i2;
                    indexBufferData[i3] = i3;

                }
            }

            if (true) {
                printf("Whatever");
            }

            for (int j = 0; j < fontMesh.GetVertexCount(); ++j) {
                difont::FontVertex vertex = fontMesh.vertices[j];
                uint32_t index = totalIndexes + j;
                vertexBufferData[index] = vertex;
                indexBufferData[index] = index;
            }
            totalIndexes += fontMesh.GetVertexCount();
        }
        DBOUT("totalIndexes: " << totalIndexes << "\n");

        if (indexBufferObject == 0)
            glGenBuffers(1, &indexBufferObject);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * totalVertexes, indexBufferData, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        free(indexBufferData);

        if (vertexBufferObject == 0)
            glGenBuffers(1, &vertexBufferObject);
        
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, sizeof(difont::FontVertex) * totalVertexes, vertexBufferData, GL_STATIC_DRAW);
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
        glVertexAttribPointer(glGetAttribLocation(shaderProgram, "Position"), 3, GL_FLOAT, GL_FALSE, sizeof(difont::FontVertex), BUFFER_OFFSET(0));
        glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "Position"));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        difont::FontMeshSet::End();
    }
}


static float f = 0.0f;
void HelloWorld::Render() {

    glBindVertexArray(vertexArrayObject);
    glDrawRangeElements(GL_TRIANGLE_STRIP, 0, totalVertexes - 1, totalIndexes, GL_UNSIGNED_INT, NULL);

    /*
    const unsigned MAX_COLORS = 4;
    color4_t colors[MAX_COLORS];
    vec4Set(colors[0], 0.8f, 0.1f, 0.0f, 1.0f);
    vec4Set(colors[1], 1.0f, 1.0f, 1.0f, 1.0f);
    vec4Set(colors[2], 0.0f, 0.1f, 0.8f, 1.0f);
    vec4Set(colors[3], 0.0f, 0.8f, 0.1f, 1.0f);

    float facesize = 2 * (sinf(f) * 0.5 + 0.5) * HELLO_WORLD_FONT_SIZE;
    m_font->FaceSize(facesize);

    f += 0.05f;
    //    glRotatef(cosf(f) * 20.0f, 0.0f, 0.0f, 1.0f);
    for (unsigned i = 0; i < MAX_COLORS; ++i) {
        color4_t color;
        vec4Set(color, colors[i][0], colors[i][1], colors[i][2], colors[i][3]);

        if (m_font) {
            difont::FontMeshSet::Begin();
            m_font->Render("hello world!");

            unsigned int fontMeshCount = difont::FontMeshSet::MeshCount();
            difont::FontMesh *meshes = difont::FontMeshSet::GetMeshes();

            printf("Meshes: %d\n", fontMeshCount);

            glPushMatrix();
            glTranslatef(0.0f, i * HELLO_WORLD_FONT_SIZE, 0.0f);
            for (int i = 0; i < fontMeshCount; ++i) {
                difont::FontMesh fontMesh = meshes[i];
                unsigned int vertexCount = fontMesh.GetVertexCount();

                glBegin(fontMesh.primitive);
                for (int j = 0; j < vertexCount; ++j) {
                    difont::FontVertex fontVertex = fontMesh.vertices[j];

                    glColor4f(color[0], color[1], color[2], color[3]);
                    glVertex3f(fontVertex.position[0], fontVertex.position[1], fontVertex.position[2]);
                    glTexCoord2f(fontVertex.texCoord[0], fontVertex.texCoord[1]);
                }
                glEnd();
            }
            glPopMatrix();
            difont::FontMeshSet::End();
        }

        if (0) { //m_textureFont) {
            difont::FontMeshSet::Begin();
            m_textureFont->Render("Hello World!");

            unsigned int fontMeshCount = difont::FontMeshSet::MeshCount();
            difont::FontMesh *meshes = difont::FontMeshSet::GetMeshes();
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_TEXTURE_2D);

            glTranslatef(0.0f, i * HELLO_WORLD_FONT_SIZE, 0.0f);
            glPushMatrix();
            for (int i = 0; i < fontMeshCount; ++i) {
                difont::FontMesh fontMesh = meshes[i];
                unsigned int vertexCount = fontMesh.GetVertexCount();

               // glActiveTexture(GL_TEXTURE0);
              //  glBindTexture(GL_TEXTURE_2D, fontMesh.textureId);
                glBegin(fontMesh.primitive);
                for (int j = 0; j < vertexCount; ++j) {
                    difont::FontVertex fontVertex = fontMesh.vertices[j];
                    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                    glVertex3f(fontVertex.position[0], fontVertex.position[1], fontVertex.position[2]);
                    glTexCoord2f(fontVertex.texCoord[0], fontVertex.texCoord[1]);
                }
                glEnd();
            }
            glPopMatrix();

            difont::FontMeshSet::End();
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
        }
    }*/
}
