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
#include "PixmapFontImpl.h"


//
//  PixmapFont
//

using namespace difont;

difont::PixmapFont::PixmapFont(char const *fontFilePath) :
    Font(new PixmapFontImpl(this, fontFilePath))
{}


difont::PixmapFont::PixmapFont(const unsigned char *pBufferBytes,
                           size_t bufferSizeInBytes) :
    Font(new PixmapFontImpl(this, pBufferBytes, bufferSizeInBytes))
{}


difont::PixmapFont::~PixmapFont()
{}


difont::Glyph* difont::PixmapFont::MakeGlyph(FT_GlyphSlot ftGlyph)
{
    return new difont::PixmapGlyph(ftGlyph);
}


//
//  PixmapFontImpl
//


difont::PixmapFontImpl::PixmapFontImpl(difont::Font *ftFont, const char* fontFilePath)
: FontImpl(ftFont, fontFilePath)
{
    load_flags = FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP;
}


difont::PixmapFontImpl::PixmapFontImpl(difont::Font *ftFont,
                                   const unsigned char *pBufferBytes,
                                   size_t bufferSizeInBytes)
: FontImpl(ftFont, pBufferBytes, bufferSizeInBytes)
{
    load_flags = FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP;
}


template <typename T>
inline difont::Point PixmapFontImpl::RenderI(const T* string, const int len,
                                         difont::Point position, difont::Point spacing,
                                         int renderMode)
{
    // Protect GL_TEXTURE_2D and GL_BLEND, glPixelTransferf(), and blending
    // functions.
	/*
    glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_COLOR_BUFFER_BIT);

    // Protect glPixelStorei() calls (made by PixmapGlyphImpl::RenderImpl).
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_TEXTURE_2D);

    GLfloat ftglColour[4];
    glGetFloatv(GL_CURRENT_RASTER_COLOR, ftglColour);

    glPixelTransferf(GL_RED_SCALE, ftglColour[0]);
    glPixelTransferf(GL_GREEN_SCALE, ftglColour[1]);
    glPixelTransferf(GL_BLUE_SCALE, ftglColour[2]);
    glPixelTransferf(GL_ALPHA_SCALE, ftglColour[3]);
*/
    difont::Point tmp = FontImpl::Render(string, len,
                                     position, spacing, renderMode);

	/*
    glPopClientAttrib();
    glPopAttrib();
*/
    return tmp;
}


difont::Point difont::PixmapFontImpl::Render(const char * string, const int len,
                                 difont::Point position, difont::Point spacing,
                                 int renderMode)
{
    return RenderI(string, len, position, spacing, renderMode);
}


difont::Point difont::PixmapFontImpl::Render(const wchar_t * string, const int len,
                                 difont::Point position, difont::Point spacing,
                                 int renderMode)
{
    return RenderI(string, len, position, spacing, renderMode);
}

