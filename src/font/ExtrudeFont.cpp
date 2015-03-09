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

#include "Internals.h"
#include "ExtrudeFontImpl.h"


//
//  ExtrudeFont
//

using namespace difont;

ExtrudeFont::ExtrudeFont(char const *fontFilePath) :
    Font(new ExtrudeFontImpl(this, fontFilePath))
{}


ExtrudeFont::ExtrudeFont(const unsigned char *pBufferBytes,
                             size_t bufferSizeInBytes) :
    Font(new ExtrudeFontImpl(this, pBufferBytes, bufferSizeInBytes))
{}


ExtrudeFont::~ExtrudeFont()
{}


Glyph* ExtrudeFont::MakeGlyph(FT_GlyphSlot ftGlyph)
{
    ExtrudeFontImpl *myimpl = dynamic_cast<ExtrudeFontImpl *>(impl);
    if(!myimpl)
    {
        return NULL;
    }

    return new ExtrudeGlyph(ftGlyph, myimpl->depth, myimpl->front,
                              myimpl->back, myimpl->useDisplayLists);
}


//
//  ExtrudeFontImpl
//


ExtrudeFontImpl::ExtrudeFontImpl(Font *ftFont, const char* fontFilePath)
: FontImpl(ftFont, fontFilePath),
  depth(0.0f), front(0.0f), back(0.0f)
{
    load_flags = FT_LOAD_NO_HINTING;
}


ExtrudeFontImpl::ExtrudeFontImpl(Font *ftFont,
                                     const unsigned char *pBufferBytes,
                                     size_t bufferSizeInBytes)
: FontImpl(ftFont, pBufferBytes, bufferSizeInBytes),
  depth(0.0f), front(0.0f), back(0.0f)
{
    load_flags = FT_LOAD_NO_HINTING;
}

