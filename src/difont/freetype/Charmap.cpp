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

#include "Face.h"
#include "Charmap.h"


difont::Charmap::Charmap(difont::Face* face)
:   ftFace(*(face->GetFTFace())),
    err(0)
{
    if(!ftFace->charmap)
    {
        if(!ftFace->num_charmaps)
        {
            // This face doesn't even have one charmap!
            err = 0x96; // Invalid_CharMap_Format
            return;
        }

        err = FT_Set_Charmap(ftFace, ftFace->charmaps[0]);
    }

    ftEncoding = ftFace->charmap->encoding;

    for(unsigned int i = 0; i < difont::Charmap::MAX_PRECOMPUTED; i++)
    {
        charIndexCache[i] = FT_Get_Char_Index(ftFace, i);
    }
}


difont::Charmap::~Charmap()
{
    charMap.clear();
}


bool difont::Charmap::CharMap(FT_Encoding encoding)
{
    if(ftEncoding == encoding)
    {
        err = 0;
        return true;
    }

    err = FT_Select_Charmap(ftFace, encoding);

    if(!err)
    {
        ftEncoding = encoding;
        charMap.clear();
    }

    return !err;
}


unsigned int difont::Charmap::GlyphListIndex(const unsigned int characterCode)
{
    return charMap[characterCode];//.find(characterCode);
}


unsigned int difont::Charmap::FontIndex(const unsigned int characterCode)
{
    if(characterCode < difont::Charmap::MAX_PRECOMPUTED)
    {
        return charIndexCache[characterCode];
    }

    return FT_Get_Char_Index(ftFace, characterCode);
}


void difont::Charmap::InsertIndex(const unsigned int characterCode,
                            const size_t containerIndex)
{
    //charMap.insert(characterCode, static_cast<GlyphIndex>(containerIndex));
    charMap[characterCode] = static_cast<GlyphIndex>(containerIndex);
}

