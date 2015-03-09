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

#ifndef _DIFONT_VECTORISER_H_
#define _DIFONT_VECTORISER_H_

#include <list>
#include <vector>
#include  <difont/difont.h>
#include "glu.h"

#include "Contour.h"

#ifndef CALLBACK
#define CALLBACK
#endif

/**
 * difont::Tesselation captures points that are output by OpenGL's gluTesselator.
 */
namespace difont {
    class Tesselation {
    public:
        /**
         * Default constructor
         */
        Tesselation(GLenum m)
        : meshType(m)
        {
            pointList.reserve(128);
        }

        /**
         *  Destructor
         */
        ~Tesselation()
        {
            pointList.clear();
        }

        /**
         * Add a point to the mesh.
         */
        void AddPoint(const FTGL_DOUBLE x, const FTGL_DOUBLE y,
                      const FTGL_DOUBLE z)
        {
            pointList.push_back(difont::Point(x, y, z));
        }

        /**
         * The number of points in this mesh
         */
        size_t PointCount() const { return pointList.size(); }

        /**
         *
         */
        const difont::Point& Point(unsigned int index) const
        { return pointList[index]; }

        /**
         * Return the OpenGL polygon type.
         */
        GLenum PolygonType() const { return meshType; }

    private:
        /**
         * Points generated by gluTesselator.
         */
        typedef std::vector<difont::Point> PointVector;
        PointVector pointList;

        /**
         * OpenGL primitive type from gluTesselator.
         */
        GLenum meshType;
    };
}


/**
 * difont::Mesh is a container of difont::Tesselation's that make up a polygon glyph
 */
namespace difont {
    class Mesh {
        typedef std::vector<difont::Tesselation*> TesselationVector;
        typedef std::list<difont::Point> PointList;

    public:
        /**
         * Default constructor
         */
        Mesh();

        /**
         *  Destructor
         */
        ~Mesh();

        /**
         * Add a point to the mesh
         */
        void AddPoint(const FTGL_DOUBLE x, const FTGL_DOUBLE y,
                      const FTGL_DOUBLE z);

        /**
         *  Create a combine point for the gluTesselator
         */
        const FTGL_DOUBLE* Combine(const FTGL_DOUBLE x, const FTGL_DOUBLE y,
                                   const FTGL_DOUBLE z);

        /**
         * Begin a new polygon
         */
        void Begin(GLenum meshType);

        /**
         * End a polygon
         */
        void End();

        /**
         * Record a gluTesselation error
         */
        void Error(GLenum e) { err = e; }

        /**
         * The number of tesselations in the mesh
         */
        size_t TesselationCount() const { return tesselationList.size(); }

        /**
         * Get a tesselation by index
         */
        const difont::Tesselation* const Tesselation(size_t index) const;

        /**
         * Return the temporary point list. For testing only.
         */
        const PointList& TempPointList() const { return tempPointList; }

        /**
         * Get the GL ERROR returned by the glu tesselator
         */
        GLenum Error() const { return err; }

    private:
        /**
         * The current sub mesh that we are constructing.
         */
        difont::Tesselation* currentTesselation;

        /**
         * Holds each sub mesh that comprises this glyph.
         */
        TesselationVector tesselationList;

        /**
         * Holds extra points created by gluTesselator. See ftglCombine.
         */
        PointList tempPointList;

        /**
         * GL ERROR returned by the glu tesselator
         */
        GLenum err;

    };
}

const FTGL_DOUBLE FTGL_FRONT_FACING = 1.0;
const FTGL_DOUBLE FTGL_BACK_FACING = -1.0;

/**
 * difont::Vectoriser class is a helper class that converts font outlines into
 * point data.
 *
 * @see ExtrudeGlyph
 * @see OutlineGlyph
 * @see PolygonGlyph
 * @see difont::Contour
 * @see difont::Point
 *
 */
namespace difont {
    class Vectoriser {
    public:
        /**
         * Constructor
         *
         * @param glyph The freetype glyph to be processed
         */
        Vectoriser(const FT_GlyphSlot glyph);

        /**
         *  Destructor
         */
        virtual ~Vectoriser();

        /**
         * Build an difont::Mesh from the vector outline data.
         *
         * @param zNormal   The direction of the z axis of the normal
         *                  for this mesh
         * FIXME: change the following for a constant
         * @param outsetType Specify the outset type contour
         *  0 : Original
         *  1 : Front
         *  2 : Back
         * @param outsetSize Specify the outset size contour
         */
        void MakeMesh(FTGL_DOUBLE zNormal = FTGL_FRONT_FACING, int outsetType = 0, float outsetSize = 0.0f);

        /**
         * Get the current mesh.
         */
        const difont::Mesh* const GetMesh() const { return mesh; }

        /**
         * Get the total count of points in this outline
         *
         * @return the number of points
         */
        size_t PointCount();

        /**
         * Get the count of contours in this outline
         *
         * @return the number of contours
         */
        size_t ContourCount() const { return ftContourCount; }

        /**
         * Return a contour at index
         *
         * @return the number of contours
         */
        const difont::Contour* const Contour(size_t index) const;

        /**
         * Get the number of points in a specific contour in this outline
         *
         * @param c     The contour index
         * @return      the number of points in contour[c]
         */
        size_t ContourSize(int c) const { return contourList[c]->PointCount(); }

        /**
         * Get the flag for the tesselation rule for this outline
         *
         * @return The contour flag
         */
        int ContourFlag() const { return contourFlag; }

    private:
        /**
         * Process the freetype outline data into contours of points
         *
         * @param front front outset distance
         * @param back back outset distance
         */
        void ProcessContours();

        /**
         * The list of contours in the glyph
         */
        difont::Contour** contourList;

        /**
         * A Mesh for tesselations
         */
        difont::Mesh* mesh;

        /**
         * The number of contours reported by Freetype
         */
        short ftContourCount;
        
        /**
         * A flag indicating the tesselation rule for the glyph
         */
        int contourFlag;
        
        /**
         * A Freetype outline
         */
        FT_Outline outline;
    };
}

#endif  //  __difont::Vectoriser__
