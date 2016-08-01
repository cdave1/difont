#include <difont/difont.h>

#ifndef _FONT_CURVE_H_
#define _FONT_CURVE_H_

namespace difont {
    enum class CurveType {
        Unspecified,
        Point,
        Line,
        Quadratic,
        Cubic
    };

    // deliberately wasteful use of memory until we have better control over paths.
    class Curve {
    public:
        Curve() {
            curveType = CurveType::Unspecified;
        }

        Curve(difont::Point a) {
            curveType = CurveType::Point;
            points[0] = a;
        }

        Curve(difont::Point a, difont::Point b) {
            curveType = CurveType::Line;
            points[0] = a;
            points[1] = b;
        }

        Curve(difont::Point a, difont::Point b, difont::Point c) {
            curveType = CurveType::Quadratic;
            points[0] = a;
            points[1] = b;
            points[2] = c;
        }

        Curve(difont::Point a, difont::Point b, difont::Point c, difont::Point d) {
            curveType = CurveType::Cubic;
            points[0] = a;
            points[1] = b;
            points[2] = c;
            points[3] = d;
        }

        difont::Point points[4];

        CurveType curveType;
    };
}

#endif