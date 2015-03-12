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

#include "Internals.h"
#include "Unicode.h"

#include "FontImpl.h"

#include "BitmapFontImpl.h"
#include "ExtrudeFontImpl.h"
#include "OutlineFontImpl.h"
#include "PixmapFontImpl.h"
#include "PolygonFontImpl.h"
#include "TextureFontImpl.h"

#include "GlyphContainer.h"
#include "Face.h"


//
//  Font
//

using namespace difont;

Font::Font(char const *fontFilePath)
{
    impl = new FontImpl(this, fontFilePath);
}


Font::Font(const unsigned char *pBufferBytes, size_t bufferSizeInBytes)
{
    impl = new FontImpl(this, pBufferBytes, bufferSizeInBytes);
}


Font::Font(FontImpl *pImpl)
{
    impl = pImpl;
}


Font::~Font()
{
    delete impl;
}


bool Font::Attach(const char* fontFilePath)
{
    return impl->Attach(fontFilePath);
}


bool Font::Attach(const unsigned char *pBufferBytes, size_t bufferSizeInBytes)
{
    return impl->Attach(pBufferBytes, bufferSizeInBytes);
}


bool Font::FaceSize(const unsigned int size, const unsigned int res)
{
    return impl->FaceSize(size, res);
}


unsigned int Font::FaceSize() const
{
    return impl->FaceSize();
}


void Font::Depth(float depth)
{
    return impl->Depth(depth);
}


void Font::Outset(float outset)
{
    return impl->Outset(outset);
}


void Font::Outset(float front, float back)
{
    return impl->Outset(front, back);
}


void Font::GlyphLoadFlags(FT_Int flags)
{
    return impl->GlyphLoadFlags(flags);
}


bool Font::CharMap(FT_Encoding encoding)
{
    return impl->CharMap(encoding);
}


unsigned int Font::CharMapCount() const
{
    return impl->CharMapCount();
}


FT_Encoding* Font::CharMapList()
{
    return impl->CharMapList();
}


void Font::UseDisplayList(bool useList)
{
    return impl->UseDisplayList(useList);
}


float Font::Ascender() const
{
    return impl->Ascender();
}


float Font::Descender() const
{
    return impl->Descender();
}


float Font::LineHeight() const
{
    return impl->LineHeight();
}


difont::Point Font::Render(const char * string, const int len,
                           difont::Point position, difont::Point spacing, int renderMode)
{
    return impl->Render(string, len, position, spacing, renderMode);
}


difont::Point Font::Render(const wchar_t * string, const int len,
                           difont::Point position, difont::Point spacing, int renderMode)
{
    return impl->Render(string, len, position, spacing, renderMode);
}



void Font::PreRender() { impl->PreRender(); }


void Font::PostRender() { impl->PostRender(); }



float Font::Advance(const char * string, const int len, difont::Point spacing)
{
    return impl->Advance(string, len, spacing);
}


float Font::Advance(const wchar_t * string, const int len, difont::Point spacing)
{
    return impl->Advance(string, len, spacing);
}


difont::BBox Font::BBox(const char *string, const int len,
                    difont::Point position, difont::Point spacing)
{
    return impl->BBox(string, len, position, spacing);
}


difont::BBox Font::BBox(const wchar_t *string, const int len,
                    difont::Point position, difont::Point spacing)
{
    return impl->BBox(string, len, position, spacing);
}


FT_Error Font::Error() const
{
    return impl->err;
}


//
//  FontImpl
//


FontImpl::FontImpl(Font *ftFont, char const *fontFilePath) :
face(fontFilePath),
useDisplayLists(true),
load_flags(FT_LOAD_DEFAULT),
intf(ftFont),
glyphList(0)
{
    err = face.Error();
    if(err == 0)
    {
        glyphList = new GlyphContainer(&face);
    }
}


FontImpl::FontImpl(Font *ftFont, const unsigned char *pBufferBytes,
                       size_t bufferSizeInBytes) :
face(pBufferBytes, bufferSizeInBytes),
useDisplayLists(true),
load_flags(FT_LOAD_DEFAULT),
intf(ftFont),
glyphList(0)
{
    err = face.Error();
    if(err == 0)
    {
        glyphList = new GlyphContainer(&face);
    }
}


FontImpl::~FontImpl()
{
    if(glyphList)
    {
        delete glyphList;
    }
}


bool FontImpl::Attach(const char* fontFilePath)
{
    if(!face.Attach(fontFilePath))
    {
        err = face.Error();
        return false;
    }

    err = 0;
    return true;
}


bool FontImpl::Attach(const unsigned char *pBufferBytes,
                        size_t bufferSizeInBytes)
{
    if(!face.Attach(pBufferBytes, bufferSizeInBytes))
    {
        err = face.Error();
        return false;
    }

    err = 0;
    return true;
}


bool FontImpl::FaceSize(const unsigned int size, const unsigned int res)
{
    if(glyphList != NULL)
    {
        delete glyphList;
        glyphList = NULL;
    }

    charSize = face.Size(size, res);
    err = face.Error();

    if(err != 0)
    {
        return false;
    }

    glyphList = new GlyphContainer(&face);
    return true;
}


unsigned int FontImpl::FaceSize() const
{
    return charSize.CharSize();
}


void FontImpl::Depth(float depth)
{
    ;
}


void FontImpl::Outset(float outset)
{
    ;
}


void FontImpl::Outset(float front, float back)
{
    ;
}


void FontImpl::GlyphLoadFlags(FT_Int flags)
{
    load_flags = flags;
}


bool FontImpl::CharMap(FT_Encoding encoding)
{
    bool result = glyphList->CharMap(encoding);
    err = glyphList->Error();
    return result;
}


unsigned int FontImpl::CharMapCount() const
{
    return face.CharMapCount();
}


FT_Encoding* FontImpl::CharMapList()
{
    return face.CharMapList();
}


void FontImpl::UseDisplayList(bool useList)
{
    useDisplayLists = useList;
}


float FontImpl::Ascender() const
{
    return charSize.Ascender();
}


float FontImpl::Descender() const
{
    return charSize.Descender();
}


float FontImpl::LineHeight() const
{
    return charSize.Height();
}


template <typename T>
inline difont::BBox FontImpl::BBoxI(const T* string, const int len,
                                difont::Point position, difont::Point spacing)
{
    difont::BBox totalBBox;

    /* Only compute the bounds if string is non-empty. */
    if(string && ('\0' != string[0]))
    {
        // for multibyte - we can't rely on sizeof(T) == character
        FTUnicodeStringItr<T> ustr(string);
        unsigned int thisChar = *ustr++;
        unsigned int nextChar = *ustr;

        if(CheckGlyph(thisChar))
        {
            totalBBox = glyphList->BBox(thisChar);
            totalBBox += position;

            position += difont::Point(glyphList->Advance(thisChar, nextChar), 0.0);
        }

        /* Expand totalBox by each glyph in string */
        for(int i = 1; (len < 0 && *ustr) || (len >= 0 && i < len); i++)
        {
            thisChar = *ustr++;
            nextChar = *ustr;

            if(CheckGlyph(thisChar))
            {
                position += spacing;

                difont::BBox tempBBox = glyphList->BBox(thisChar);
                tempBBox += position;
                totalBBox |= tempBBox;

                position += difont::Point(glyphList->Advance(thisChar, nextChar),
                                    0.0);
            }
        }
    }

    return totalBBox;
}


difont::BBox FontImpl::BBox(const char *string, const int len,
                        difont::Point position, difont::Point spacing)
{
    /* The chars need to be unsigned because they are cast to int later */
    return BBoxI((const unsigned char *)string, len, position, spacing);
}


difont::BBox FontImpl::BBox(const wchar_t *string, const int len,
                        difont::Point position, difont::Point spacing)
{
    return BBoxI(string, len, position, spacing);
}


template <typename T>
inline float FontImpl::AdvanceI(const T* string, const int len,
                                  difont::Point spacing)
{
    float advance = 0.0f;
    FTUnicodeStringItr<T> ustr(string);

    for(int i = 0; (len < 0 && *ustr) || (len >= 0 && i < len); i++)
    {
        unsigned int thisChar = *ustr++;
        unsigned int nextChar = *ustr;

        if(CheckGlyph(thisChar))
        {
            advance += glyphList->Advance(thisChar, nextChar);
        }

        if(nextChar)
        {
            advance += spacing.Xf();
        }
    }

    return advance;
}


float FontImpl::Advance(const char* string, const int len, difont::Point spacing)
{
    /* The chars need to be unsigned because they are cast to int later */
    return AdvanceI((const unsigned char *)string, len, spacing);
}


float FontImpl::Advance(const wchar_t* string, const int len, difont::Point spacing)
{
    return AdvanceI(string, len, spacing);
}


template <typename T>
inline difont::Point FontImpl::RenderI(const T* string, const int len,
                                   difont::Point position, difont::Point spacing,
                                   int renderMode)
{
    // for multibyte - we can't rely on sizeof(T) == character
    FTUnicodeStringItr<T> ustr(string);

    for(int i = 0; (len < 0 && *ustr) || (len >= 0 && i < len); i++)
    {
        unsigned int thisChar = *ustr++;
        unsigned int nextChar = *ustr;

        if(CheckGlyph(thisChar))
        {

            position += glyphList->Render(thisChar, nextChar,
                                          position, renderMode);
        }

        if(nextChar)
        {
            position += spacing;
        }
    }

    return position;
}


difont::Point FontImpl::Render(const char * string, const int len,
                           difont::Point position, difont::Point spacing, int renderMode)
{
    difont::Point tmp = FontImpl::RenderI(string, len, position, spacing, renderMode);
    return tmp;
}


difont::Point FontImpl::Render(const wchar_t * string, const int len,
                           difont::Point position, difont::Point spacing, int renderMode)
{
    difont::Point tmp = FontImpl::RenderI(string, len, position, spacing, renderMode);
    return tmp;
}


bool FontImpl::CheckGlyph(const unsigned int characterCode)
{
    if(glyphList->Glyph(characterCode))
    {
        return true;
    }

    unsigned int glyphIndex = glyphList->FontIndex(characterCode);
    FT_GlyphSlot ftSlot = face.Glyph(glyphIndex, load_flags);
    if(!ftSlot)
    {
        err = face.Error();
        return false;
    }

    Glyph* tempGlyph = intf->MakeGlyph(ftSlot);
    if(!tempGlyph)
    {
        if(0 == err)
        {
            err = 0x13;
        }
        
        return false;
    }
    
    glyphList->Add(tempGlyph, characterCode);
    
    return true;
}

