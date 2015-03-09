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

#include "OpenGLShim.h"
#include "CommonMath.h"

void difont::examples::OpenGL::Begin(GLenum prim) {
    glBegin(prim);
}


void difont::examples::OpenGL::Vertex3f(float x, float y, float z) {
    glVertex3f(x, y, z);
}


void difont::examples::OpenGL::Color4f(float r, float g, float b, float a) {
    glColor4f(r, g, b, a);
}


void difont::examples::OpenGL::TexCoord2f(float s, float t) {
    glTexCoord2f(s, t);
}


void difont::examples::OpenGL::End() {
    glEnd();
}
