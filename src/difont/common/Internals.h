/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 * Copyright (c) 2008 Éric Beets <ericbeets@free.fr>
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

#ifndef _DIFONT_INTERNALS_H_
#define _DIFONT_INTERNALS_H_

#include <difont/difont.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/*
// Fixes for deprecated identifiers in 2.1.5
#ifndef FT_OPEN_MEMORY
    #define FT_OPEN_MEMORY (FT_Open_Flags)1
#endif

#ifndef FT_RENDER_MODE_MONO
    #define FT_RENDER_MODE_MONO ft_render_mode_mono
#endif

#ifndef FT_RENDER_MODE_NORMAL
    #define FT_RENDER_MODE_NORMAL ft_render_mode_normal
#endif
*/

#include "difont/opengl/OpenGLInterface.h"

namespace difont {
    typedef enum
    {
        GLYPH_CUSTOM,
        GLYPH_BITMAP,
        GLYPH_BUFFER,
        GLYPH_PIXMAP,
        GLYPH_OUTLINE,
        GLYPH_POLYGON,
        GLYPH_EXTRUDE,
        GLYPH_TEXTURE,
    } GlyphType;

    struct _FTGLglyph
    {
        Glyph *ptr;
        GlyphType type;
    };

    typedef enum
    {
        FONT_CUSTOM,
        FONT_BITMAP,
        FONT_BUFFER,
        FONT_PIXMAP,
        FONT_OUTLINE,
        FONT_POLYGON,
        FONT_EXTRUDE,
        FONT_TEXTURE,
    } FontType;

    struct _FTGLfont
    {
        Font *ptr;
        FontType type;
    };
    
    typedef enum
    {
        LAYOUT_SIMPLE,
    } LayoutType;
    
    struct _FTGLlayout
    {
        Layout *ptr;
        Font *font;
        LayoutType type;
    };
}

#endif  //__FTINTERNALS_H__

