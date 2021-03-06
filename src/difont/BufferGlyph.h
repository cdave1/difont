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

#include <difont/difont.h>

#ifndef _DIFONT_BUFFER_GLYPH_H_
#define _DIFONT_BUFFER_GLYPH_H_

/**
 * BufferGlyph is a specialisation of Glyph for memory buffer rendering.
 */
namespace difont {
    class BufferGlyph : public Glyph {
    public:
        /**
         * Constructor
         *
         * @param glyph The Freetype glyph to be processed
         * @param buffer  An Buffer object in which to render the glyph.
         */
        BufferGlyph(FT_GlyphSlot glyph, difont::Buffer *buffer);

        /**
         * Destructor
         */
        virtual ~BufferGlyph();

        /**
         * Render this glyph at the current pen position.
         *
         * @param pen  The current pen position.
         * @param renderMode  Render mode to display
         * @return  The advance distance for this glyph.
         */
        virtual const difont::Point& Render(const difont::Point& pen, int renderMode);
    };
}

#endif  //  __BufferGlyph__

