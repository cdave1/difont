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

#ifndef _DIFONT_OPEN_GL_INTERFACE_H_
#define _DIFONT_OPEN_GL_INTERFACE_H_

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#ifdef WIN32
#include <windows.h>
#include "gl.h"
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

#ifndef GL_QUADS
#define GL_QUADS 888
#endif

#ifdef __cplusplus
extern "C" {
#endif
	
	extern void ftglBegin(unsigned int prim);
	
	extern void ftglVertex3f(float x, float y, float z);
	
	extern void ftglVertex2f(float x, float y);
	
	extern void ftglColor4f(float r, float g, float b, float a );
		
	extern void ftglTexCoord2f(float s, float t );
	
    extern void ftglBindTexture(unsigned int textureId);
    
	extern void ftglEnd();
	
	extern void ftglError(const char *source);

    extern uint32_t ftglVertexSize();

    extern uint32_t ftglVertexCount();

    extern void ftglCopyMesh(void *dataPointer, void *dataLen, uint32_t *vertexCount);
	
#ifdef __cplusplus
}
#endif

#endif