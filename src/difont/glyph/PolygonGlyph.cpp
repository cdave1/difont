/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 * Copyright (c) 2008 Éric Beets <ericbeets@free.fr>
 * Copyright (c) 2008 Sam Hocevar <sam@zoy.org>
 * Copyright (c) 2014 David Petrie <david@davidpetrie.com>
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
#include "PolygonGlyphImpl.h"
#include "Vectoriser.h"

using namespace difont;

PolygonGlyph::PolygonGlyph(FT_GlyphSlot glyph, float outset,
                               bool useDisplayList) :
    Glyph(new PolygonGlyphImpl(glyph, outset, useDisplayList))
{}


PolygonGlyph::~PolygonGlyph()
{}


const difont::Point& PolygonGlyph::Render(const difont::Point& pen, int renderMode)
{
    PolygonGlyphImpl *myimpl = dynamic_cast<PolygonGlyphImpl *>(impl);
    return myimpl->RenderImpl(pen, renderMode);
}


const difont::Point& PolygonGlyph::Render(const difont::Point& pen, difont::RenderData &renderData, int renderMode)
{
    PolygonGlyphImpl *myimpl = dynamic_cast<PolygonGlyphImpl *>(impl);
    return myimpl->RenderImpl(pen, renderData, renderMode);
}


PolygonGlyphImpl::PolygonGlyphImpl(FT_GlyphSlot glyph, float _outset,
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

    if ((vectoriser->ContourCount() < 1) || (vectoriser->PointCount() < 3))
    {
        delete vectoriser;
        vectoriser = NULL;
        return;
    }

    hscale = glyph->face->size->metrics.x_ppem * 64;
    vscale = glyph->face->size->metrics.y_ppem * 64;
    outset = _outset;

	if (vectoriser)
		vectoriser->MakeMesh(1.0, 1, outset);
}


PolygonGlyphImpl::~PolygonGlyphImpl()
{
	if (vectoriser)
    {
        delete vectoriser;
    }
}


const difont::Point& PolygonGlyphImpl::RenderImpl(const difont::Point& pen,
                                                  int renderMode) {
    
}


const difont::Point& PolygonGlyphImpl::RenderImpl(const difont::Point& pen,
                                                  difont::RenderData &renderData, int renderMode) {
	if (vectoriser) {
		DoRender(pen, renderData);
	}
    return advance;
}


void PolygonGlyphImpl::DoRender(const difont::Point& pen, difont::RenderData &renderData) {
    difont::GlyphData glyphData;

    glyphData.SetBaseline(pen);

    difont::Point lower(pen.X() +  this->BBox().Lower().X(),
                        pen.Y() + -this->BBox().Lower().Y());
    difont::Point upper(pen.X() +  this->BBox().Upper().X(),
                        pen.Y() + -this->BBox().Upper().Y());

    glyphData.SetBoundingBox(difont::BBox(lower, upper));
    
    for(unsigned int c = 0; c < vectoriser->ContourCount(); ++c) {
        const difont::Contour *contour = vectoriser->Contour(c);
        difont::Path path;
        path.AddContour(contour, pen, 1.0 / 64.0);
        glyphData.AddPath(path);
    }

    const difont::Mesh *mesh = vectoriser->GetMesh();

    for(unsigned int t = 0; t < mesh->TesselationCount(); ++t) {
        const difont::Tesselation* subMesh = mesh->Tesselation(t);
        unsigned int polygonType = subMesh->PolygonType();

        difont::FontMesh mesh(polygonType);

		for(unsigned int i = 0; i < subMesh->PointCount(); ++i) {
			difont::Point point = subMesh->Point(i);

            difont::Point point1 = difont::Point(point.X() + outset,
                                                 point.Y() + outset,
                                                 0);

            difont::FontVertex vertex;
            vertex.SetTexCoord2f(point1.Xf() / hscale, point.Yf() / vscale);
            vertex.SetVertex3f(pen.Xf() + point.Xf() / 64.0f, pen.Yf() + point.Yf() / 64.0f, 0.0f);
            mesh.AddVertex(vertex);
		}

        glyphData.AddMesh(mesh);
    }
    renderData.AddGlyph(glyphData);
}

