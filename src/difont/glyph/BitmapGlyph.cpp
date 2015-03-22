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

#include <string>

#include  <difont/difont.h>

#include "Internals.h"
#include "BitmapGlyphImpl.h"


//
//  FTGLBitmapGlyph
//

using namespace difont;

BitmapGlyph::BitmapGlyph(FT_GlyphSlot glyph) :
    Glyph(new BitmapGlyphImpl(glyph))
{}


BitmapGlyph::~BitmapGlyph()
{}


const difont::Point& BitmapGlyph::Render(const difont::Point& pen, int renderMode)
{
    BitmapGlyphImpl *myimpl = dynamic_cast<BitmapGlyphImpl *>(impl);
    return myimpl->RenderImpl(pen, renderMode);
}


//
//  FTGLBitmapGlyphImpl
//


BitmapGlyphImpl::BitmapGlyphImpl(FT_GlyphSlot glyph)
:   GlyphImpl(glyph),
    destWidth(0),
    destHeight(0),
    data(0)
{
    err = FT_Render_Glyph(glyph, FT_RENDER_MODE_MONO);
    if (err || ft_glyph_format_bitmap != glyph->format)
    {
        return;
    }

    FT_Bitmap bitmap = glyph->bitmap;

    unsigned int srcWidth = bitmap.width;
    unsigned int srcHeight = bitmap.rows;
    unsigned int srcPitch = bitmap.pitch;

    destWidth = srcWidth;
    destHeight = srcHeight;
    destPitch = srcPitch;

    if (destWidth && destHeight)
    {
        data = new unsigned char[destPitch * destHeight];
        unsigned char* dest = data + ((destHeight - 1) * destPitch);

        unsigned char* src = bitmap.buffer;

        for(unsigned int y = 0; y < srcHeight; ++y)
        {
            memcpy(dest, src, srcPitch);
            dest -= destPitch;
            src += srcPitch;
        }
    }

    pos = difont::Point(glyph->bitmap_left, static_cast<int>(srcHeight) - glyph->bitmap_top, 0.0);
}


BitmapGlyphImpl::~BitmapGlyphImpl()
{
    delete [] data;
}


const difont::Point& BitmapGlyphImpl::RenderImpl(const difont::Point& pen, int renderMode)
{
    if(data)
    {
        float dx, dy;

        dx = pen.Xf() + pos.Xf();
        dy = pen.Yf() - pos.Yf();
/*
        glBitmap(0, 0, 0.0f, 0.0f, dx, dy, (const GLubyte*)0);
        glPixelStorei(GL_PACK_ALIGNMENT, destPitch * 8);
        glBitmap(destWidth, destHeight, 0.0f, 0.0, 0.0, 0.0,
                 (const GLubyte*)data);
        glBitmap(0, 0, 0.0f, 0.0f, -dx, -dy, (const GLubyte*)0);
		*/
		glTexSubImage2D(GL_TEXTURE_2D,
						0,
						0.0f, 0.0f,
						destWidth, destHeight,
						GL_RGBA, GL_UNSIGNED_BYTE,
						(const GLubyte*)data);
    }

    return advance;
}

