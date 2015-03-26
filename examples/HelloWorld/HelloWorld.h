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

#ifndef _HELLO_WORLD_H_
#define _HELLO_WORLD_H_

#include <stdint.h>
#include <difont/difont.h>
#include <difont/opengl/OpenGLInterface.h>

class HelloWorld {
private:
    difont::Font *m_font;

    difont::Font *m_textureFont;

    float m_width, m_height, m_scale;

public:
    HelloWorld(float width, float height, float scale) : m_width(width), m_height(height), m_scale(scale) {}
    
    ~HelloWorld() {}
    
    void SetupFonts(const char *fontpath);

    void SetupVertexArrays(GLuint shaderProgram);
    
	void Update(GLuint shaderProgram);

    void Render(GLuint shaderProgram);
    
};

#endif