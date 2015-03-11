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

#ifndef _DIFONT_CONTOUR_H_
#define _DIFONT_CONTOUR_H_

#include  <difont/difont.h>
#include <vector>

/**
 * difont::Contour class is a container of points that describe a vector font
 * outline. It is used as a container for the output of the bezier curve
 * evaluator in difont::Vectoriser.
 *
 * @see OutlineGlyph
 * @see PolygonGlyph
 * @see difont::Point
 */
namespace difont {
    class Contour {
    public:
        /**
         * Constructor
         *
         * @param contour
         * @param pointTags
         * @param numberOfPoints
         */
        Contour(FT_Vector* contour, char* pointTags, unsigned int numberOfPoints);

        /**
         * Destructor
         */
        ~Contour()
        {
            pointList.clear();
            outsetPointList.clear();
            frontPointList.clear();
            backPointList.clear();
        }

        /**
         * Return a point at index.
         *
         * @param index of the point in the curve.
         * @return const point reference
         */
        const difont::Point& Point(size_t index) const { return pointList[index]; }

        /**
         * Return a point at index.
         *
         * @param index of the point in the outset curve.
         * @return const point reference
         */
        const difont::Point& Outset(size_t index) const { return outsetPointList[index]; }

        /**
         * Return a point at index of the front outset contour.
         *
         * @param index of the point in the curve.
         * @return const point reference
         */
        const difont::Point& FrontPoint(size_t index) const
        {
            if(frontPointList.size() == 0)
                return Point(index);
            return frontPointList[index];
        }


        /**
         * Return a point at index of the back outset contour.
         *
         * @param index of the point in the curve.
         * @return const point reference
         */
        const difont::Point& BackPoint(size_t index) const
        {
            if(backPointList.size() == 0)
                return Point(index);
            return backPointList[index];
        }

        /**
         * How many points define this contour
         *
         * @return the number of points in this contour
         */
        size_t PointCount() const { return pointList.size(); }

        /**
         * Make sure the glyph has the proper parity and create the front/back
         * outset contour.
         *
         * @param parity  The contour's parity within the glyph.
         */
        void SetParity(int parity);

        // FIXME: this should probably go away.
        void buildFrontOutset(float outset);
        void buildBackOutset(float outset);

    private:
        /**
         * Add a point to this contour. This function tests for duplicate
         * points.
         *
         * @param point The point to be added to the contour.
         */
        inline void AddPoint(difont::Point point);

        /**
         * Add a point to this contour. This function tests for duplicate
         * points.
         *
         * @param point The point to be added to the contour.
         */
        inline void AddOutsetPoint(difont::Point point);

        /*
         * Add a point to this outset contour. This function tests for duplicate
         * points.
         *
         * @param point The point to be added to the contour outset.
         */
        inline void AddFrontPoint(difont::Point point);
        inline void AddBackPoint(difont::Point point);

        /**
         * De Casteljau (bezier) algorithm contributed by Jed Soane
         * Evaluates a quadratic or conic (second degree) curve
         */
        inline void evaluateQuadraticCurve(difont::Point, difont::Point, difont::Point);

        /**
         * De Casteljau (bezier) algorithm contributed by Jed Soane
         * Evaluates a cubic (third degree) curve
         */
        inline void evaluateCubicCurve(difont::Point, difont::Point, difont::Point, difont::Point);

        /**
         * Compute the vector norm
         */
        inline FTGL_DOUBLE NormVector(const difont::Point &v);

        /**
         * Compute a rotation matrix from a vector
         */
        inline void RotationMatrix(const difont::Point &a, const difont::Point &b, FTGL_DOUBLE *matRot, FTGL_DOUBLE *invRot);

        /**
         * Matrix and vector multiplication
         */
        inline void MultMatrixVect(FTGL_DOUBLE *mat, difont::Point &v);

        /**
         * Compute the vector bisecting from a vector 'v' and a distance 'd'
         */
        inline void ComputeBisec(difont::Point &v);

        /**
         * Compute the outset point coordinates
         */
        inline difont::Point ComputeOutsetPoint(difont::Point a, difont::Point b, difont::Point c);

        /**
         *  The list of points in this contour
         */
        typedef std::vector<difont::Point> PointVector;
        PointVector pointList;
        PointVector outsetPointList;
        PointVector frontPointList;
        PointVector backPointList;
        
        /**
         *  Is this contour clockwise or anti-clockwise?
         */
        bool clockwise;
    };
}

#endif // __difont::Contour__

