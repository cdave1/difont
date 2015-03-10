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

#include <math.h>

#include  <difont/difont.h>

#include "Internals.h"
#include "PixmapGlyphImpl.h"


//
//  FTGLPixmapGlyph
//


difont::PixmapGlyph::PixmapGlyph(FT_GlyphSlot glyph) :
Glyph(new PixmapGlyphImpl(glyph))
{}


difont::PixmapGlyph::~PixmapGlyph()
{}


const difont::Point& difont::PixmapGlyph::Render(const difont::Point& pen, int renderMode)
{
    PixmapGlyphImpl *myimpl = dynamic_cast<PixmapGlyphImpl *>(impl);
    return myimpl->RenderImpl(pen, renderMode);
}


//
//  FTGLPixmapGlyphImpl
//


difont::PixmapGlyphImpl::PixmapGlyphImpl(FT_GlyphSlot glyph)
:   GlyphImpl(glyph),
destWidth(0),
destHeight(0),
data(0)
{
    err = FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);
    if(err || ft_glyph_format_bitmap != glyph->format)
    {
        return;
    }

    FT_Bitmap bitmap = glyph->bitmap;

    //check the pixel mode
    //ft_pixel_mode_grays

    int srcWidth = bitmap.width;
    int srcHeight = bitmap.rows;

    destWidth = srcWidth;
    destHeight = srcHeight;

    if(destWidth && destHeight)
    {
        data = new unsigned char[destWidth * destHeight * 2];
        unsigned char* src = bitmap.buffer;

        unsigned char* dest = data + ((destHeight - 1) * destWidth * 2);
        size_t destStep = destWidth * 2 * 2;

        for(int y = 0; y < srcHeight; ++y)
        {
            for(int x = 0; x < srcWidth; ++x)
            {
                *dest++ = static_cast<unsigned char>(255);
                *dest++ = *src++;
            }
            dest -= destStep;
        }

        destHeight = srcHeight;
    }

    pos.X(glyph->bitmap_left);
    pos.Y(srcHeight - glyph->bitmap_top);
}


difont::PixmapGlyphImpl::~PixmapGlyphImpl()
{
    delete [] data;
}


const difont::Point& difont::PixmapGlyphImpl::RenderImpl(const difont::Point& pen,
                                                         int renderMode)
{
    if(data)
    {
        float dx, dy;

        dx = floor(pen.Xf() + pos.Xf());
        dy = floor(pen.Yf() - pos.Yf());

        /*
         glBitmap(0, 0, 0.0f, 0.0f, dx, dy, (const GLubyte*)0);
         glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
         glPixelStorei(GL_UNPACK_ALIGNMENT, 2);

         glDrawPixels(destWidth, destHeight, GL_LUMINANCE_ALPHA,
         GL_UNSIGNED_BYTE, (const GLvoid*)data);
         glBitmap(0, 0, 0.0f, 0.0f, -dx, -dy, (const GLubyte*)0);
         */
    }
    
    return advance;
}

