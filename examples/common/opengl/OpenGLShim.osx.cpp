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
#include <fstream>
#include <string>
#include <sstream>

void difont::examples::OpenGL::Begin(GLenum prim) {
    //glBegin(prim);
}


void difont::examples::OpenGL::Vertex3f(float x, float y, float z) {
    //glVertex3f(x, y, z);
}


void difont::examples::OpenGL::Color4f(float r, float g, float b, float a) {
    //glColor4f(r, g, b, a);
}


void difont::examples::OpenGL::TexCoord2f(float s, float t) {
    //glTexCoord2f(s, t);
}


void difont::examples::OpenGL::End() {
    //glEnd();
}

static std::string m_shaderInfoLog;
static std::string m_programInfoLog;

GLuint difont::examples::OpenGL::loadFragmentShader(const char *path) {
    return difont::examples::OpenGL::loadShader(path, GL_FRAGMENT_SHADER);
}


GLuint difont::examples::OpenGL::loadVertexShader(const char *path) {
    return difont::examples::OpenGL::loadShader(path, GL_VERTEX_SHADER);
}


GLuint difont::examples::OpenGL::loadShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath) {
    GLint compiled = 0;
    GLint linked = 0;
    GLuint vertexShader = 0;
    GLuint fragmentShader = 0;
    GLuint shaderProgram = 0;

    vertexShader = loadVertexShader(vertexShaderPath);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        m_shaderInfoLog = shaderInfoLog(vertexShader);
        glDeleteShader(vertexShader);

        std::ostringstream output;

        output << "Error compiling GLSL vertex shader: '";
        output << vertexShaderPath;
        output << "'" << std::endl << std::endl;
        output << "Shader info log:" << std::endl;
        output << m_shaderInfoLog;

        m_shaderInfoLog = output.str();
        fprintf(stderr, "%s\n", m_shaderInfoLog.c_str());
    }
    else
    {
        fragmentShader = loadFragmentShader(fragmentShaderPath);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);

        if (!compiled)
        {
            m_shaderInfoLog = shaderInfoLog(fragmentShader);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            std::ostringstream output;

            output << "Error compiling GLSL fragment shader: '";
            output << fragmentShaderPath;
            output << "'" << std::endl << std::endl;
            output << "Shader info log:" << std::endl;
            output << m_shaderInfoLog;

            m_shaderInfoLog = output.str();
            fprintf(stderr, "%s\n", m_shaderInfoLog.c_str());
        }
        else
        {
            shaderProgram = glCreateProgram();
            glAttachShader(shaderProgram, vertexShader);
            glAttachShader(shaderProgram, fragmentShader);
            glLinkProgram(shaderProgram);
            glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
            
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            if (!linked)
            {
                m_programInfoLog = programInfoLog(shaderProgram);
                glDeleteProgram(shaderProgram);
                
                std::ostringstream output;
                
                output << "Error linking GLSL shaders into a shader program." << std::endl;
                output << "GLSL vertex shader: '" << vertexShaderPath;
                output << "'" << std::endl;
                output << "GLSL fragment shader: '" << fragmentShaderPath;
                output << "'" << std::endl << std::endl;
                output << "Program info log:" << std::endl;
                output << m_programInfoLog;
                
                m_programInfoLog = output.str();
                fprintf(stderr, "%s\n", m_programInfoLog.c_str());
            }
        }
    }
    
    return shaderProgram;
}

GLuint difont::examples::OpenGL::loadShader(const char *filename, GLenum shaderType) {
    std::string source;
    std::ifstream file(filename, std::ios::binary);

    if (file.is_open())
    {
        file.seekg(0, std::ios::end);

        unsigned int fileSize = static_cast<unsigned int>(file.tellg());

        source.resize(fileSize);
        file.seekg(0, std::ios::beg);
        file.read(&source[0], fileSize);
    }

    GLuint shader = glCreateShader(shaderType);
    const GLchar *pszSource = reinterpret_cast<const GLchar *>(source.c_str());
    
    glShaderSource(shader, 1, &pszSource, 0);
    
    return shader;
}


std::string difont::examples::OpenGL::programInfoLog(GLuint program)
{
    GLsizei infoLogSize = 0;
    std::string infoLog;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogSize);
    infoLog.resize(infoLogSize);
    glGetProgramInfoLog(program, infoLogSize, &infoLogSize, &infoLog[0]);

    return infoLog;
}

std::string difont::examples::OpenGL::shaderInfoLog(GLuint shader)
{
    GLsizei infoLogSize = 0;
    std::string infoLog;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogSize);
    infoLog.resize(infoLogSize);
    glGetShaderInfoLog(shader, infoLogSize, &infoLogSize, &infoLog[0]);
    
    return infoLog;
}
