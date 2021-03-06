/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 * Copyright (c) 2008 Sam Hocevar <sam@zoy.org>
 * Copyright (c) 2008 Sean Morrison <learner@brlcad.org>
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

#ifndef _DIFONT_POINT_H_
#define _DIFONT_POINT_H_

/**
 * difont::Point class is a basic 3-dimensional point or vector.
 */
namespace difont {
    class Point {
    public:
        /**
         * Default constructor. Point is set to zero.
         */
        inline Point()
        {
            values[0] = 0;
            values[1] = 0;
            values[2] = 0;
        }

        /**
         * Constructor. Z coordinate is set to zero if unspecified.
         *
         * @param x First component
         * @param y Second component
         * @param z Third component
         */
        inline Point(const double x, const double y,
                     const double z = 0)
        {
            values[0] = x;
            values[1] = y;
            values[2] = z;
        }

        /**
         * Constructor. This converts an FT_Vector to an difont::Point
         *
         * @param ft_vector A freetype vector
         */
        inline Point(const FT_Vector& ft_vector)
        {
            values[0] = ft_vector.x;
            values[1] = ft_vector.y;
            values[2] = 0;
        }

        /**
         * Normalise a point's coordinates. If the coordinates are zero,
         * the point is left untouched.
         *
         * @return A vector of norm one.
         */
        Point Normalise();


        /**
         * Operator += In Place Addition.
         *
         * @param point
         * @return this plus point.
         */
        inline Point& operator += (const difont::Point& point)
        {
            values[0] += point.values[0];
            values[1] += point.values[1];
            values[2] += point.values[2];

            return *this;
        }

        /**
         * Operator +
         *
         * @param point
         * @return this plus point.
         */
        inline Point operator + (const difont::Point& point) const
        {
            difont::Point temp;
            temp.values[0] = values[0] + point.values[0];
            temp.values[1] = values[1] + point.values[1];
            temp.values[2] = values[2] + point.values[2];

            return temp;
        }

        /**
         * Operator -= In Place Substraction.
         *
         * @param point
         * @return this minus point.
         */
        inline Point& operator -= (const difont::Point& point)
        {
            values[0] -= point.values[0];
            values[1] -= point.values[1];
            values[2] -= point.values[2];

            return *this;
        }

        /**
         * Operator -
         *
         * @param point
         * @return this minus point.
         */
        inline Point operator - (const Point& point) const
        {
            difont::Point temp;
            temp.values[0] = values[0] - point.values[0];
            temp.values[1] = values[1] - point.values[1];
            temp.values[2] = values[2] - point.values[2];

            return temp;
        }

        /**
         * Operator *  Scalar multiplication
         *
         * @param multiplier
         * @return <code>this</code> multiplied by <code>multiplier</code>.
         */
        inline Point operator * (double multiplier) const
        {
            difont::Point temp;
            temp.values[0] = values[0] * multiplier;
            temp.values[1] = values[1] * multiplier;
            temp.values[2] = values[2] * multiplier;

            return temp;
        }


        /**
         * Operator *  Scalar multiplication
         *
         * @param point
         * @param multiplier
         * @return <code>multiplier</code> multiplied by <code>point</code>.
         */
        inline friend Point operator * (double multiplier, Point& point)
        {
            return point * multiplier;
        }


        /**
         * Operator *  Scalar product
         *
         * @param a  First vector.
         * @param b  Second vector.
         * @return  <code>a.b</code> scalar product.
         */
        inline friend double operator * (Point &a, Point& b)
        {
            return a.values[0] * b.values[0]
            + a.values[1] * b.values[1]
            + a.values[2] * b.values[2];
        }


        /**
         * Operator ^  Vector product
         *
         * @param point Second point
         * @return this vector point.
         */
        inline Point operator ^ (const Point& point)
        {
            difont::Point temp;
            temp.values[0] = values[1] * point.values[2]
            - values[2] * point.values[1];
            temp.values[1] = values[2] * point.values[0]
            - values[0] * point.values[2];
            temp.values[2] = values[0] * point.values[1]
            - values[1] * point.values[0];
            return temp;
        }


        /**
         * Operator == Tests for equality
         *
         * @param a
         * @param b
         * @return true if a & b are equal
         */
        friend bool operator == (const Point &a, const Point &b);


        /**
         * Operator != Tests for non equality
         *
         * @param a
         * @param b
         * @return true if a & b are not equal
         */
        friend bool operator != (const Point &a, const Point &b);


        /**
         * Cast to double*
         */
        inline operator const double*() const
        {
            return values;
        }


        /**
         * Setters
         */
        inline void X(double x) { values[0] = x; };
        inline void Y(double y) { values[1] = y; };
        inline void Z(double z) { values[2] = z; };


        /**
         * Getters
         */
        inline double X() const { return values[0]; };
        inline double Y() const { return values[1]; };
        inline double Z() const { return values[2]; };
        inline float Xf() const { return static_cast<float>(values[0]); };
        inline float Yf() const { return static_cast<float>(values[1]); };
        inline float Zf() const { return static_cast<float>(values[2]); };
        
    private:
        /**
         * The point data
         */
        double values[3];
    };

    bool operator == (const Point &a, const Point &b);
    bool operator != (const Point &a, const Point &b);
}

#endif
