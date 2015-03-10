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

#include  <difont/difont.h>

#include "Internals.h"
#include "BitmapFontImpl.h"


//
//  BitmapFont
//

using namespace difont;

BitmapFont::BitmapFont(char const *fontFilePath) :
    Font(new BitmapFontImpl(this, fontFilePath))
{}


BitmapFont::BitmapFont(unsigned char const *pBufferBytes,
                           size_t bufferSizeInBytes) :
    Font(new BitmapFontImpl(this, pBufferBytes, bufferSizeInBytes))
{}


BitmapFont::~BitmapFont()
{}


Glyph* BitmapFont::MakeGlyph(FT_GlyphSlot ftGlyph)
{
    return new BitmapGlyph(ftGlyph);
}


//
//  BitmapFontImpl
//


template <typename T>
inline difont::Point BitmapFontImpl::RenderI(const T* string, const int len,
                                         difont::Point position, difont::Point spacing,
                                         int renderMode)
{
    // Protect GL_BLEND
   // glPushAttrib(GL_COLOR_BUFFER_BIT);

    // Protect glPixelStorei() calls (also in BitmapGlyphImpl::RenderImpl)
    //glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

   // glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glDisable(GL_BLEND);

    difont::Point tmp = FontImpl::Render(string, len,
                                     position, spacing, renderMode);

   // glPopClientAttrib();
    //glPopAttrib();

    return tmp;
}


difont::Point BitmapFontImpl::Render(const char * string, const int len,
                                 difont::Point position, difont::Point spacing,
                                 int renderMode)
{
    return RenderI(string, len, position, spacing, renderMode);
}


difont::Point BitmapFontImpl::Render(const wchar_t * string, const int len,
                                 difont::Point position, difont::Point spacing,
                                 int renderMode)
{
    return RenderI(string, len, position, spacing, renderMode);
}

