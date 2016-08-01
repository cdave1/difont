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

#ifndef _DIFONT_BufferFontImpl__
#define _DIFONT_BufferFontImpl__

#include "FontImpl.h"

namespace difont {

    class Glyph;
    class Buffer;
    
    class BufferFontImpl : public FontImpl
    {
        friend class BufferFont;

    protected:
        BufferFontImpl(Font *ftFont, const char* fontFilePath);

        BufferFontImpl(Font *ftFont, const unsigned char *pBufferBytes,
                         size_t bufferSizeInBytes);

        virtual ~BufferFontImpl();

        virtual difont::Point Render(const char *s, const int len,
                                     difont::Point position, difont::Point spacing,
                                     int renderMode);

        virtual difont::Point Render(const wchar_t *s, const int len,
                                     difont::Point position, difont::Point spacing,
                                     int renderMode);

        virtual bool FaceSize(const unsigned int size,
                              const unsigned int res);

    private:
        /**
         * Create an BufferGlyph object for the base class.
         */
        Glyph* MakeGlyphImpl(FT_GlyphSlot ftGlyph);

        /* Internal generic Render() implementation */
        template <typename T>
        inline difont::Point RenderI(const T *s, const int len,
                                     difont::Point position, difont::Point spacing, int mode);

        template <typename T>
        inline difont::Point RenderI(const T *s, difont::RenderData &renderData, const int len,
                                     difont::Point position, difont::Point spacing, int mode);
        /* Pixel buffer */
        Buffer *buffer;

        static const int BUFFER_CACHE_SIZE = 64;
        /* Texture IDs */
        GLuint idCache[BUFFER_CACHE_SIZE];
        void *stringCache[BUFFER_CACHE_SIZE];
        difont::BBox bboxCache[BUFFER_CACHE_SIZE];
        difont::Point advanceCache[BUFFER_CACHE_SIZE];
        int lastString;
    };
}

#endif  //  __BufferFontImpl__

