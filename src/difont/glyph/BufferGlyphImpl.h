/*
 * FTGL - OpenGL font library
 *
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

#ifndef _DIFONT_BUFFER_GLYPH_IMPL_H_
#define _DIFONT_BUFFER_GLYPH_IMPL_H_

#include "GlyphImpl.h"

namespace difont {
    class BufferGlyphImpl : public GlyphImpl {
        friend class BufferGlyph;

    protected:
        BufferGlyphImpl(FT_GlyphSlot glyph, Buffer *p);

        virtual ~BufferGlyphImpl();

        virtual const difont::Point& RenderImpl(const difont::Point& pen, int renderMode);

    private:
        bool has_bitmap;
        FT_Bitmap bitmap;
        unsigned char *pixels;
        difont::Point corner;

        Buffer *buffer;
    };
}

#endif  //  __BufferGlyphImpl__

