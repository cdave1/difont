/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 * Copyright (c) 2008 Sam Hocevar <sam@zoy.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <cassert>
#include <string> // For memset

#include  <difont/difont.h>

#include "Internals.h"

#include "../glyph/TextureGlyphImpl.h"
#include "./TextureFontImpl.h"


//
//  difont::TextureFont
//


difont::TextureFont::TextureFont(char const *fontFilePath) :
    Font(new difont::TextureFontImpl(this, fontFilePath))
{}


difont::TextureFont::TextureFont(const unsigned char *pBufferBytes,
                             size_t bufferSizeInBytes) :
    Font(new difont::TextureFontImpl(this, pBufferBytes, bufferSizeInBytes))
{}


difont::TextureFont::~TextureFont()
{}


difont::Glyph* difont::TextureFont::MakeGlyph(FT_GlyphSlot ftGlyph)
{
    difont::TextureFontImpl *myimpl = dynamic_cast<difont::TextureFontImpl *>(impl);
    if(!myimpl)
    {
        return NULL;
    }

    return myimpl->MakeGlyphImpl(ftGlyph);
}


//
//  difont::TextureFontImpl
//


static inline GLuint NextPowerOf2(GLuint in)
{
     in -= 1;

     in |= in >> 16;
     in |= in >> 8;
     in |= in >> 4;
     in |= in >> 2;
     in |= in >> 1;

     return in + 1;
}


difont::TextureFontImpl::TextureFontImpl(Font *ftFont, const char* fontFilePath)
:   FontImpl(ftFont, fontFilePath),
    maximumGLTextureSize(0),
    textureWidth(0),
    textureHeight(0),
    glyphHeight(0),
    glyphWidth(0),
    padding(3),
    xOffset(0),
    yOffset(0)
{
    load_flags = FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP;
    remGlyphs = numGlyphs = face.GlyphCount();
	preRendered = false;
}


difont::TextureFontImpl::TextureFontImpl(Font *ftFont,
                                     const unsigned char *pBufferBytes,
                                     size_t bufferSizeInBytes)
:   FontImpl(ftFont, pBufferBytes, bufferSizeInBytes),
    maximumGLTextureSize(0),
    textureWidth(0),
    textureHeight(0),
    glyphHeight(0),
    glyphWidth(0),
    padding(3),
    xOffset(0),
    yOffset(0)
{
    load_flags = FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP;
    remGlyphs = numGlyphs = face.GlyphCount();
}


difont::TextureFontImpl::~TextureFontImpl()
{
    if(textureIDList.size())
    {
        glDeleteTextures((GLsizei)textureIDList.size(),
                         (const GLuint*)&textureIDList[0]);
    }
}


difont::Glyph* difont::TextureFontImpl::MakeGlyphImpl(FT_GlyphSlot ftGlyph)
{
    glyphHeight = static_cast<int>(charSize.Height() + 0.5);
    glyphWidth = static_cast<int>(charSize.Width() + 0.5);

    if(glyphHeight < 1) glyphHeight = 1;
    if(glyphWidth < 1) glyphWidth = 1;
	
    if(textureIDList.empty())
    {
        textureIDList.push_back(CreateTexture());
        xOffset = yOffset = padding;
    }

    if(xOffset > (textureWidth - glyphWidth))
    {
        xOffset = padding;
        yOffset += glyphHeight;

        if(yOffset > (textureHeight - glyphHeight))
        {
            textureIDList.push_back(CreateTexture());
            yOffset = padding;
        }
    }

    TextureGlyph* tempGlyph = new TextureGlyph(ftGlyph, textureIDList[textureIDList.size() - 1],
                                                    xOffset, yOffset, textureWidth, textureHeight);
    xOffset += static_cast<int>(tempGlyph->BBox().Upper().X() - tempGlyph->BBox().Lower().X() + padding + 0.5);
	
	--remGlyphs;

    return tempGlyph;
}


void difont::TextureFontImpl::CalculateTextureSize() {
    if (!maximumGLTextureSize) {
        maximumGLTextureSize = 512;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&maximumGLTextureSize);
        assert(maximumGLTextureSize);
    }
    maximumGLTextureSize = 2048;
    textureWidth = NextPowerOf2((remGlyphs * glyphWidth) + (padding * 2));
    textureWidth = textureWidth > maximumGLTextureSize ? maximumGLTextureSize : textureWidth;

    int h = static_cast<int>((textureWidth - (padding * 2)) / glyphWidth + 0.5);

    textureHeight = NextPowerOf2(((numGlyphs / h) + 1) * glyphHeight);
    textureHeight = textureHeight > maximumGLTextureSize ? maximumGLTextureSize : textureHeight;
}


GLuint difont::TextureFontImpl::CreateTexture() {
	CalculateTextureSize();

    int totalMemory = textureWidth * textureHeight;
    unsigned char *textureMemory = (unsigned char *)calloc(1, sizeof(unsigned char) * textureWidth * textureHeight);
	
    GLuint textID;
    glGenTextures(1, (GLuint*)&textID);

    ftglBindTexture(textID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, textureWidth, textureHeight,
                 0, GL_ALPHA, GL_UNSIGNED_BYTE, textureMemory);
	
	printf("texture dimensions: %d %d\n", textureWidth, textureHeight);
	
    delete [] textureMemory;

    return textID;
}


bool difont::TextureFontImpl::FaceSize(const unsigned int size, const unsigned int res)
{
    if(!textureIDList.empty())
    {
        glDeleteTextures((GLsizei)textureIDList.size(), (const GLuint*)&textureIDList[0]);
        textureIDList.clear();
        remGlyphs = numGlyphs = face.GlyphCount();
    }

    return FontImpl::FaceSize(size, res);
}


template <typename T>
inline difont::Point difont::TextureFontImpl::RenderI(const T* string, const int len,
                                          difont::Point position, difont::Point spacing,
                                          int renderMode)
{
	disableTexture2D = false;
	disableBlend = false;
    difont::Point tmp = FontImpl::Render(string, len, position, spacing, renderMode);
    return tmp;
}


void difont::TextureFontImpl::PreRender() 
{
}


void difont::TextureFontImpl::PostRender() 
{
}


difont::Point difont::TextureFontImpl::Render(const char * string, const int len,
                                  difont::Point position, difont::Point spacing,
                                  int renderMode)
{
    return RenderI(string, len, position, spacing, renderMode);
}


difont::Point difont::TextureFontImpl::Render(const wchar_t * string, const int len,
                                  difont::Point position, difont::Point spacing,
                                  int renderMode)
{
    
    return RenderI(string, len, position, spacing, renderMode);
}

