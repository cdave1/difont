#ifndef _FONT_CURVES_H_
#define _FONT_CURVES_H_

namespace difont {
    enum class CurveType {
        Unspecified,
        Line,
        Quadratic,
        Cubic
    };

    class CurvePoint {
    public:
        CurvePoint() {
            position[0] = position[1] = position[2] = 0.0f;
        }

        CurvePoint(float x, float y, float z) {
            position[0] = x;
            position[1] = y;
            position[2] = z;
        }

        void SetVertex3f(float x, float y, float z) {
            position[0] = x;
            position[1] = y;
            position[2] = z;
        }

        float position[3];
    };


    class Curve {
    public:
        Curve() {
            curveType = CurveType::Unspecified;
        }

        Curve(CurvePoint a, CurvePoint b) {
            curveType = CurveType::Line;
            points[0] = a;
            points[1] = b;
        }

        Curve(CurvePoint a, CurvePoint b, CurvePoint c) {
            curveType = CurveType::Quadratic;
            points[0] = a;
            points[1] = b;
            points[2] = c;
        }

        Curve(CurvePoint a, CurvePoint b, CurvePoint c, CurvePoint d) {
            curveType = CurveType::Cubic;
            points[0] = a;
            points[1] = b;
            points[2] = c;
            points[3] = d;
        }

        CurvePoint points[4];

        CurveType curveType;
    };


    class Path {
    public:
        void Path();

        void AddLine(CurvePoint a, CurvePoint b);

        void AddQuadratic(CurvePoint a, CurvePoint b, CurvePoint c);

        void AddCubic(CurvePoint a, CurvePoint b, CurvePoint c, CurvePoint d);

        void End();

        int CurveCount();

        difont::Curve * GetCurves();
    };
}

#endif