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

#ifndef GLES2_RENDER_CONTROLLER_H
#define GLES2_RENDER_CONTROLLER_H

#if TARGET_OS_IPHONE == 1 || TARGET_IPHONE_SIMULATOR == 1
#define TARGET_GL_OPENGLES 1
#elif TARGET_OS_MAC == 1
#define TARGET_GL_OPENGL 1
#else
#define TARGET_GL_OPENGL 1
#endif

#if TARGET_GL_OPENGLES == 1
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#define GL_QUADS 888
typedef double GLdouble;
#elif TARGET_GL_OPENGL == 1
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#endif

namespace difont {
    namespace examples {
        class OpenGL {

            static void Begin(GLenum prim);

            static void Vertex3f(float x, float y, float z);

            static void Color4f(float r, float g, float b, float a);

            static void TexCoord2f(float s, float t);
            
            static void End();
            
            static void Error(const char *source);
            
        };
    }
}

#endif