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

void HelloWorld::SetupFonts(const char *fontpath) {
    m_font = new difont::PolygonFont(fontpath);
    if (m_font->Error()) {
        fprintf(stderr, "Could not load font at `%s'\n", fontpath);
        delete m_font;
        m_font = NULL;
    } else {
        const uint32_t HELLO_WORLD_FONT_SIZE = 400;
        m_font->FaceSize(HELLO_WORLD_FONT_SIZE);
        m_font->CharMap(FT_ENCODING_ADOBE_LATIN_1);
    }

    m_textureFont = new difont::TextureFont(fontpath);
    if (m_textureFont->Error()) {
        fprintf(stderr, "Could not load font at `%s'\n", fontpath);
        delete m_textureFont;
        m_textureFont = NULL;
    } else {
        const uint32_t HELLO_WORLD_FONT_SIZE = 400;
        m_textureFont->FaceSize(HELLO_WORLD_FONT_SIZE);
        m_textureFont->CharMap(FT_ENCODING_ADOBE_LATIN_1);
    }
}


void HelloWorld::Render() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, m_width * m_scale, 0.0f, m_height * m_scale, 0.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (m_font) {
        difont::FontMeshSet::Begin();
        m_font->Render("Hello World!");

        unsigned int fontMeshCount = difont::FontMeshSet::MeshCount();
        difont::FontMesh *meshes = difont::FontMeshSet::GetMeshes();
        glTranslatef(0.0f, 200.0f, 0.0f);
        for (int i = 0; i < fontMeshCount; ++i) {
            difont::FontMesh fontMesh = meshes[i];
            unsigned int vertexCount = fontMesh.GetVertexCount();

            glBegin(fontMesh.primitive);
            for (int j = 0; j < vertexCount; ++j) {
                difont::FontVertex fontVertex = fontMesh.vertices[j];

                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glVertex3f(fontVertex.position[0], fontVertex.position[1], fontVertex.position[2]);
                glTexCoord2f(fontVertex.texCoord[0], fontVertex.texCoord[1]);
            }
            glEnd();
        }

        difont::FontMeshSet::End();
    }

    /*
    if (m_textureFont) {
        difont::FontMeshSet::Begin();
        m_textureFont->Render("Hello World!");

        unsigned int fontMeshCount = difont::FontMeshSet::MeshCount();
        difont::FontMesh *meshes = difont::FontMeshSet::GetMeshes();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);
        glTranslatef(0.0f, 400.0f, 0.0f);
        for (int i = 0; i < fontMeshCount; ++i) {
            difont::FontMesh fontMesh = meshes[i];
            unsigned int vertexCount = fontMesh.GetVertexCount();

glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fontMesh.textureId);
            glBegin(fontMesh.primitive);
            for (int j = 0; j < vertexCount; ++j) {
                difont::FontVertex fontVertex = fontMesh.vertices[j];
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glVertex3f(fontVertex.position[0], fontVertex.position[1], fontVertex.position[2]);
                glTexCoord2f(fontVertex.texCoord[0], fontVertex.texCoord[1]);
            }
            glEnd();
        }
        
        difont::FontMeshSet::End();

        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);

        glVertex2f(0.0f, 500.0f);
        glTexCoord2f(0.0f, 1.0f);

        glVertex2f(1000.0f, 500.0f);
        glTexCoord2f(1.0f, 1.0f);

        glVertex2f(1000.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f);

        glEnd();

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }*/
}
