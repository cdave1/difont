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

#ifndef DIFONT_OPEN_GL_SHIM_H
#define DIFONT_OPEN_GL_SHIM_H

#ifdef WIN32
#include <windows.h>
#include <gl/GLU.h>
#include <GL/glu.h>
#define ftglprintf(...) printf(__VA_ARGS__)
#elif TARGET_OS_IPHONE == 1 || TARGET_IPHONE_SIMULATOR == 1
#define ftglprintf(...) printf(__VA_ARGS__)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#elif ANDROID == 1
#include <GLES/glplatform.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <jni.h>
#define ftglprintf(...) ((void)__android_log_print(ANDROID_LOG_INFO, "TestApp", __VA_ARGS__))
#else
#define ftglprintf(...) printf(__VA_ARGS__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#endif

#include <string>

namespace difont {
    namespace examples {
        class OpenGL {
        public:
            static void Begin(GLenum prim);

            static void Vertex3f(float x, float y, float z);

            static void Color4f(float r, float g, float b, float a);

            static void TexCoord2f(float s, float t);
            
            static void End();
            
            static void Error(const char *source);

            static GLuint loadFragmentShader(const char *path);

            static GLuint loadVertexShader(const char *path);

            static GLuint loadShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath);

            static GLuint loadShader(const char *filename, GLenum shaderType);

            static std::string programInfoLog(GLuint program);

            static std::string shaderInfoLog(GLuint shader);
        };
    }
}

#endif