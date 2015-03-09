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

#include  <difont/difont.h>

#include "GlyphContainer.h"
#include "Face.h"
#include "Charmap.h"


difont::GlyphContainer::GlyphContainer(Face* f)
:   face(f),
err(0)
{
    glyphs.push_back(NULL);
    charMap = new difont::Charmap(face);
}


difont::GlyphContainer::~GlyphContainer()
{
    GlyphVector::iterator it;
    for(it = glyphs.begin(); it != glyphs.end(); ++it)
    {
        delete *it;
    }

    glyphs.clear();
    delete charMap;
}


bool difont::GlyphContainer::CharMap(FT_Encoding encoding)
{
    bool result = charMap->CharMap(encoding);
    err = charMap->Error();
    return result;
}


unsigned int difont::GlyphContainer::FontIndex(const unsigned int charCode) const
{
    return charMap->FontIndex(charCode);
}


void difont::GlyphContainer::Add(difont::Glyph* tempGlyph, const unsigned int charCode)
{
    charMap->InsertIndex(charCode, glyphs.size());
    glyphs.push_back(tempGlyph);
}


const difont::Glyph* const difont::GlyphContainer::Glyph(const unsigned int charCode) const
{
    unsigned int index = charMap->GlyphListIndex(charCode);
    return glyphs[index];
}


difont::BBox difont::GlyphContainer::BBox(const unsigned int charCode) const
{
    return Glyph(charCode)->BBox();
}


float difont::GlyphContainer::Advance(const unsigned int charCode,
                                      const unsigned int nextCharCode)
{
    unsigned int left = charMap->FontIndex(charCode);
    unsigned int right = charMap->FontIndex(nextCharCode);

    return face->KernAdvance(left, right).Xf() + Glyph(charCode)->Advance();
}


difont::Point difont::GlyphContainer::Render(const unsigned int charCode,
                                             const unsigned int nextCharCode,
                                             difont::Point penPosition, int renderMode)
{
    unsigned int left = charMap->FontIndex(charCode);
    unsigned int right = charMap->FontIndex(nextCharCode);

    difont::Point kernAdvance = face->KernAdvance(left, right);

    if(!face->Error())
    {
        unsigned int index = charMap->GlyphListIndex(charCode);
        kernAdvance += glyphs[index]->Render(penPosition, renderMode);
    }
    
    return kernAdvance;
}

