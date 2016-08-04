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

#include  <difont/difont.h>

#include "Internals.h"
#include "OutlineGlyphImpl.h"
#include "Vectoriser.h"


//
//  FTGLOutlineGlyph
//
using namespace difont;

OutlineGlyph::OutlineGlyph(FT_GlyphSlot glyph, float outset,
                               bool useDisplayList) :
    Glyph(new OutlineGlyphImpl(glyph, outset, useDisplayList))
{}


OutlineGlyph::~OutlineGlyph()
{}


const difont::Point& OutlineGlyph::Render(const difont::Point& pen, difont::RenderData &renderData, int renderMode)
{
    OutlineGlyphImpl *myimpl = dynamic_cast<OutlineGlyphImpl *>(impl);
    return myimpl->RenderImpl(pen, renderData, renderMode);
}


//
//  FTGLOutlineGlyphImpl
//


OutlineGlyphImpl::OutlineGlyphImpl(FT_GlyphSlot glyph, float _outset,
                                       bool useDisplayList)
:   GlyphImpl(glyph),
    glList(0)
{
    if(ft_glyph_format_outline != glyph->format)
    {
        err = 0x14; // Invalid_Outline
        return;
    }

    vectoriser = new difont::Vectoriser(glyph);

    if((vectoriser->ContourCount() < 1) || (vectoriser->PointCount() < 3))
    {
        delete vectoriser;
        vectoriser = NULL;
        return;
    }

    outset = _outset;
}


OutlineGlyphImpl::~OutlineGlyphImpl()
{
	if (vectoriser)
    {
        delete vectoriser;
    }
}


const difont::Point& OutlineGlyphImpl::RenderImpl(const difont::Point& pen, difont::RenderData &renderData,
                                              int renderMode)
{
    if(vectoriser)
    {
        RenderContours(pen, renderData);
    }

    return advance;
}


void OutlineGlyphImpl::RenderContours(const difont::Point& pen, difont::RenderData &renderData) {
    difont::GlyphData glyphData;
    for(unsigned int c = 0; c < vectoriser->ContourCount(); ++c) {
        const difont::Contour *contour = vectoriser->Contour(c);
        difont::Path path;
        path.AddPath(contour->GetPath(), pen, contour->Clockwise());
        glyphData.AddPath(path);
    }

    difont::FontMesh mesh(GL_LINES);

	for(unsigned int c = 0; c < vectoriser->ContourCount(); ++c)
    {
        const difont::Contour* contour = vectoriser->Contour(c);
		
		for(unsigned int i = 0; i < contour->PointCount(); ++i)
		{
			unsigned ii = (i+1 == contour->PointCount()) ? 0 : i+1;
			
			difont::Point point1 = difont::Point(contour->Point(i).X() + contour->Outset(i).X() * outset,
									 contour->Point(i).Y() + contour->Outset(i).Y() * outset,
									 0);
			difont::Point point2 = difont::Point(contour->Point(ii).X() + contour->Outset(ii).X() * outset,
									 contour->Point(ii).Y() + contour->Outset(ii).Y() * outset,
									 0);

            difont::FontVertex vertex1;
            vertex1.SetVertex2f((point1.Xf() / 64.0f) + pen.Xf(),
						        (point1.Yf() / 64.0f) + pen.Yf());
            mesh.AddVertex(vertex1);

            difont::FontVertex vertex2;
            vertex2.SetVertex2f((point2.Xf() / 64.0f) + pen.Xf(),
						        (point2.Yf() / 64.0f) + pen.Yf());
            mesh.AddVertex(vertex2);
        }
        glyphData.AddMesh(mesh);
    }
    renderData.AddGlyph(glyphData);
}

