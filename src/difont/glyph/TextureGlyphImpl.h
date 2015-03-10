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

#ifndef _DIFONT_TextureGlyphImpl__
#define _DIFONT_TextureGlyphImpl__

#include "GlyphImpl.h"

namespace difont {
    class TextureGlyphImpl : public GlyphImpl {
        friend class TextureGlyph;
        friend class TextureFontImpl;

    protected:
        TextureGlyphImpl(FT_GlyphSlot glyph, int id, int xOffset,
                           int yOffset, int width, int height);

        virtual ~TextureGlyphImpl();

        virtual const difont::Point& RenderImpl(const difont::Point& pen, int renderMode);

    private:
        /**
         * Kept for backwards compatability with FTGL.
         */
        static void ResetActiveTexture() {}

        /**
         * The width of the glyph 'image'
         */
        int destWidth;

        /**
         * The height of the glyph 'image'
         */
        int destHeight;

        /**
         * Vector from the pen position to the topleft corner of the pixmap
         */
        difont::Point corner;

        /**
         * The texture co-ords of this glyph within the texture.
         */
        difont::Point uv[2];

        /**
         * The texture index that this glyph is contained in.
         */
        int glTextureID;
    };
}

#endif  //  __TextureGlyphImpl__

