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
#include "OutlineFontImpl.h"

using namespace difont;

OutlineFont::OutlineFont(char const *fontFilePath) :
    Font(new OutlineFontImpl(this, fontFilePath))
{}


OutlineFont::OutlineFont(const unsigned char *pBufferBytes,
                             size_t bufferSizeInBytes) :
    Font(new OutlineFontImpl(this, pBufferBytes, bufferSizeInBytes))
{}


OutlineFont::~OutlineFont()
{}


Glyph* OutlineFont::MakeGlyph(FT_GlyphSlot ftGlyph)
{
    OutlineFontImpl *myimpl = dynamic_cast<OutlineFontImpl *>(impl);
    if(!myimpl)
    {
        return NULL;
    }

    return new OutlineGlyph(ftGlyph, myimpl->outset,
                              myimpl->useDisplayLists);
}


//
//  OutlineFontImpl
//


OutlineFontImpl::OutlineFontImpl(Font *ftFont, const char* fontFilePath)
: FontImpl(ftFont, fontFilePath),
  outset(0.0f)
{
    load_flags = FT_LOAD_NO_HINTING;
	preRendered = false;
}


OutlineFontImpl::OutlineFontImpl(Font *ftFont,
                                     const unsigned char *pBufferBytes,
                                     size_t bufferSizeInBytes)
: FontImpl(ftFont, pBufferBytes, bufferSizeInBytes),
  outset(0.0f)
{
    load_flags = FT_LOAD_NO_HINTING;
	preRendered = false;
}
/*

template <typename T>
inline difont::Point OutlineFontImpl::RenderI(const T* string, const int len,
                                          difont::Point position, difont::Point spacing,
                                          int renderMode)
{
	difont::Point tmp;
    difont::RenderData::Begin();
    difont::RenderData::AddMesh(GL_LINES);
	if (preRendered)
	{
		tmp = FontImpl::Render(string, len, position, spacing, renderMode);
	}
	else 
	{
		PreRender();
		tmp = FontImpl::Render(string, len, position, spacing, renderMode);
		PostRender();
	}
    difont::RenderData::End();
    return tmp;
}


void OutlineFontImpl::PreRender()
{
	preRendered = true;
}


void OutlineFontImpl::PostRender()
{
	preRendered = false;
}


difont::Point OutlineFontImpl::Render(const char * string, const int len,
                                  difont::Point position, difont::Point spacing,
                                  int renderMode)
{
    return RenderI(string, len, position, spacing, renderMode);
}


difont::Point OutlineFontImpl::Render(const wchar_t * string, const int len,
                                  difont::Point position, difont::Point spacing,
                                  int renderMode)
{
    return RenderI(string, len, position, spacing, renderMode);
}*/

