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

#include "Internals.h"
#include "Vectoriser.h"
#include <OpenGL/glu.h>

#ifndef CALLBACK
#define CALLBACK
#endif

#if defined __APPLE_CC__ && __APPLE_CC__ < 5465
    typedef GLvoid (*GLUTesselatorFunction) (...);
#elif defined WIN32 && !defined __CYGWIN__
    typedef GLvoid (CALLBACK *GLUTesselatorFunction) ();
#else
    typedef GLvoid (*GLUTesselatorFunction) ();
#endif


void CALLBACK ftglErrorTess(GLenum errCode, difont::Mesh* mesh)
{
    mesh->Error(errCode);
}


void CALLBACK ftglVertexTess(void* data, difont::Mesh* mesh)
{
    double* vertex = static_cast<double*>(data);
    mesh->AddPoint(vertex[0], vertex[1], vertex[2]);
}


void CALLBACK ftglCombineTess(double coords[3], void* vertex_data[4], GLfloat weight[4], void** outData, difont::Mesh* mesh)
{
    const double* vertex = static_cast<const double*>(coords);
    *outData = const_cast<double*>(mesh->Combine(vertex[0], vertex[1], vertex[2]));
}

void CALLBACK ftglBeginTess(GLenum type, difont::Mesh* mesh)
{
    mesh->Begin(type);
}


void CALLBACK ftglEndTess(difont::Mesh* mesh)
{
    mesh->End();
}


difont::Mesh::Mesh()
: currentTesselation(0),
    err(0)
{
    tesselationList.reserve(16);
}


difont::Mesh::~Mesh()
{
    for(size_t t = 0; t < tesselationList.size(); ++t)
    {
        delete tesselationList[t];
    }

    tesselationList.clear();
}


void difont::Mesh::AddPoint(const double x, const double y, const double z)
{
    currentTesselation->AddPoint(x, y, z);
}


const double* difont::Mesh::Combine(const double x, const double y, const double z)
{
    tempPointList.push_back(difont::Point(x, y,z));
    return static_cast<const double*>(tempPointList.back());
}


void difont::Mesh::Begin(GLenum meshType)
{
    currentTesselation = new difont::Tesselation(meshType);
}


void difont::Mesh::End()
{
    tesselationList.push_back(currentTesselation);
}


const difont::Tesselation* const difont::Mesh::Tesselation(size_t index) const
{
    return (index < tesselationList.size()) ? tesselationList[index] : NULL;
}


difont::Vectoriser::Vectoriser(const FT_GlyphSlot glyph)
:   contourList(0),
    mesh(0),
    ftContourCount(0),
    contourFlag(0)
{
    if(glyph)
    {
        outline = glyph->outline;

        ftContourCount = outline.n_contours;
        contourList = 0;
        contourFlag = outline.flags;

        ProcessContours();
    }
}


difont::Vectoriser::~Vectoriser()
{
    for(size_t c = 0; c < ContourCount(); ++c)
    {
        delete contourList[c];
    }

    delete [] contourList;
    delete mesh;
}


void difont::Vectoriser::ProcessContours()
{
    short contourLength = 0;
    short startIndex = 0;
    short endIndex = 0;

    contourList = new difont::Contour*[ftContourCount];

    for(int i = 0; i < ftContourCount; ++i)
    {
        FT_Vector* pointList = &outline.points[startIndex];
        char* tagList = &outline.tags[startIndex];

        endIndex = outline.contours[i];
        contourLength =  (endIndex - startIndex) + 1;

        difont::Contour* contour = new difont::Contour(pointList, tagList, contourLength);

        contourList[i] = contour;

        startIndex = endIndex + 1;
    }

    // Compute each contour's parity. FIXME: see if FT_Outline_Get_Orientation
    // can do it for us.
    for(int i = 0; i < ftContourCount; i++)
    {
        difont::Contour *c1 = contourList[i];

        // 1. Find the leftmost point.
        difont::Point leftmost(65536.0, 0.0);

        for(size_t n = 0; n < c1->PointCount(); n++)
        {
            difont::Point p = c1->Point(n);
            if(p.X() < leftmost.X())
            {
                leftmost = p;
            }
        }

        // 2. Count how many other contours we cross when going further to
        // the left.
        int parity = 0;

        for(int j = 0; j < ftContourCount; j++)
        {
            if(j == i)
            {
                continue;
            }

            difont::Contour *c2 = contourList[j];

            for(size_t n = 0; n < c2->PointCount(); n++)
            {
                difont::Point p1 = c2->Point(n);
                difont::Point p2 = c2->Point((n + 1) % c2->PointCount());

                /* FIXME: combinations of >= > <= and < do not seem stable */
                if((p1.Y() < leftmost.Y() && p2.Y() < leftmost.Y())
                    || (p1.Y() >= leftmost.Y() && p2.Y() >= leftmost.Y())
                    || (p1.X() > leftmost.X() && p2.X() > leftmost.X()))
                {
                    continue;
                }
                else if(p1.X() < leftmost.X() && p2.X() < leftmost.X())
                {
                    parity++;
                }
                else
                {
                    difont::Point a = p1 - leftmost;
                    difont::Point b = p2 - leftmost;
                    if(b.X() * a.Y() > b.Y() * a.X())
                    {
                        parity++;
                    }
                }
            }
        }

        // 3. Make sure the glyph has the proper parity.
        c1->SetParity(parity);
    }
}


size_t difont::Vectoriser::PointCount()
{
    size_t s = 0;
    for(size_t c = 0; c < ContourCount(); ++c)
    {
        s += contourList[c]->PointCount();
    }

    return s;
}


const difont::Contour* const difont::Vectoriser::Contour(size_t index) const
{
    return (index < ContourCount()) ? contourList[index] : NULL;
}

void difont::Vectoriser::MakeMesh(double zNormal, int outsetType, float outsetSize)
{
    if(mesh)
    {
        delete mesh;
    }

    mesh = new difont::Mesh;
	
    GLUtesselator* tobj = gluNewTess();

    gluTessCallback(tobj, GLU_TESS_BEGIN_DATA,     (GLUTesselatorFunction)ftglBeginTess);
    gluTessCallback(tobj, GLU_TESS_VERTEX_DATA,    (GLUTesselatorFunction)ftglVertexTess);
    gluTessCallback(tobj, GLU_TESS_COMBINE_DATA,   (GLUTesselatorFunction)ftglCombineTess);
    gluTessCallback(tobj, GLU_TESS_END_DATA,       (GLUTesselatorFunction)ftglEndTess);
    gluTessCallback(tobj, GLU_TESS_ERROR_DATA,     (GLUTesselatorFunction)ftglErrorTess);

    if(contourFlag & ft_outline_even_odd_fill) // ft_outline_reverse_fill
    {
        gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
    }
    else
    {
        gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);
    }
	

    gluTessProperty(tobj, GLU_TESS_TOLERANCE, 0);
    gluTessNormal(tobj, 0.0f, 0.0f, zNormal);
    gluTessBeginPolygon(tobj, mesh);

        for(size_t c = 0; c < ContourCount(); ++c)
        {
            /* Build the */
	
            switch(outsetType)
            {
                case 1 : contourList[c]->buildFrontOutset(outsetSize); break;
                case 2 : contourList[c]->buildBackOutset(outsetSize); break;
            }
            const difont::Contour* contour = contourList[c];


           gluTessBeginContour(tobj);
                for(size_t p = 0; p < contour->PointCount(); ++p)
                {
                    const double* d;
                    switch(outsetType)
                    {
                        case 1: d = contour->FrontPoint(p); break;
                        case 2: d = contour->BackPoint(p); break;
                        case 0: default: d = contour->Point(p); break;
                    }
                    // XXX: gluTessVertex doesn't modify the data but does not
                    // specify "const" in its prototype, so we cannot cast to
                    // a const type.
                    gluTessVertex(tobj, (GLdouble *)d, (GLvoid *)d);
                }

            gluTessEndContour(tobj);
        }
    gluTessEndPolygon(tobj);

	gluDeleteTess(tobj);
}

