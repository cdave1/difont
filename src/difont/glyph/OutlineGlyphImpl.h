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

#ifndef _DIFONT_OUTLINE_GLYPH_IMPL_H_
#define _DIFONT_OUTLINE_GLYPH_IMPL_H_

#include "GlyphImpl.h"

namespace difont {
    class Vectoriser;

    class OutlineGlyphImpl : public GlyphImpl {
        friend class OutlineGlyph;

    protected:
        OutlineGlyphImpl(FT_GlyphSlot glyph, float outset,
                         bool useDisplayList);

        virtual ~OutlineGlyphImpl();

        virtual const difont::Point& RenderImpl(const difont::Point& pen, int renderMode);

        virtual const difont::Point& RenderImpl(const difont::Point& pen, difont::RenderData &renderData, int renderMode);

    private:
        /**
         * Renders contours of vectoriser.
         */
        void RenderContours(const difont::Point& pen, difont::RenderData &renderData);

        /**
         * Private rendering variables.
         */
        difont::Vectoriser *vectoriser;

        difont::Path *path;

        /**
         * Private rendering variables.
         */
        float outset;

        /**
         * OpenGL display list
         */
        GLuint glList;
    };
}

#endif
