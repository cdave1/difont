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

#ifndef _DIFONT_FontImpl__
#define _DIFONT_FontImpl__

#include  <difont/difont.h>

#include "Face.h"

namespace difont {
    class GlyphContainer;
    class Glyph;

    class FontImpl {
        friend class Font;
    protected:
        FontImpl(Font *ftFont, char const *fontFilePath);

        FontImpl(Font *ftFont, const unsigned char *pBufferBytes,
                 size_t bufferSizeInBytes);

        virtual ~FontImpl();

        virtual bool Attach(const char* fontFilePath);

        virtual bool Attach(const unsigned char *pBufferBytes,
                            size_t bufferSizeInBytes);

        virtual void GlyphLoadFlags(FT_Int flags);

        virtual bool CharMap(FT_Encoding encoding);

        virtual unsigned int CharMapCount() const;

        virtual FT_Encoding* CharMapList();

        virtual void UseDisplayList(bool useList);

        virtual float Ascender() const;

        virtual float Descender() const;

        virtual float LineHeight() const;

        virtual bool FaceSize(const unsigned int size,
                              const unsigned int res);

        virtual unsigned int FaceSize() const;

        virtual void Depth(float depth);

        virtual void Outset(float outset);

        virtual void Outset(float front, float back);

        virtual difont::BBox BBox(const char *s, const int len, difont::Point, difont::Point);

        virtual difont::BBox BBox(const wchar_t *s, const int len, difont::Point, difont::Point);

        virtual float Advance(const char *s, const int len, difont::Point);

        virtual float Advance(const wchar_t *s, const int len, difont::Point);

        virtual difont::Point Render(const char *s, const int len,
                                     difont::Point, difont::Point, int);

        virtual difont::Point Render(const wchar_t *s, const int len,
                                     difont::Point, difont::Point, int);

        virtual void PreRender() {}


        virtual	void PostRender() {}

        /**
         * Current face object
         */
        Face face;

        /**
         * Current size object
         */
        difont::Size charSize;

        /**
         * Flag to enable or disable the use of Display Lists inside FTGL
         * <code>true</code> turns ON display lists.
         * <code>false</code> turns OFF display lists.
         */
        bool useDisplayLists;

        /**
         * The default glyph loading flags.
         */
        FT_Int load_flags;

        /**
         * Current error code. Zero means no error.
         */
        FT_Error err;

    private:
        /**
         * A link back to the interface of which we are the implementation.
         */
        Font *intf;

        /**
         * Check that the glyph at <code>chr</code> exist. If not load it.
         *
         * @param chr  character index
         * @return <code>true</code> if the glyph can be created.
         */
        bool CheckGlyph(const unsigned int chr);

        /**
         * An object that holds a list of glyphs
         */
        GlyphContainer* glyphList;

        /**
         * Current pen or cursor position;
         */
        difont::Point pen;

        /* Internal generic BBox() implementation */
        template <typename T>
        inline difont::BBox BBoxI(const T *s, const int len,
                                  difont::Point position, difont::Point spacing);
        
        /* Internal generic Advance() implementation */
        template <typename T>
        inline float AdvanceI(const T *s, const int len, difont::Point spacing);
        
        /* Internal generic Render() implementation */
        template <typename T>
        inline difont::Point RenderI(const T *s, const int len,
                                     difont::Point position, difont::Point spacing, int mode);
    };
}

#endif  //  __FontImpl__

