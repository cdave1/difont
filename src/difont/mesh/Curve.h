#include <difont/difont.h>

#ifndef _FONT_CURVE_H_
#define _FONT_CURVE_H_

#include <string>

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

        std::string ToSVG() const {
            static char output[128];
            if (curveType == difont::CurveType::Point) {
                snprintf(output,
                         sizeof(output),
                         "L%f,%f ",
                         points[0].X(),
                         points[0].Y());
            } else if (curveType == difont::CurveType::Line) {
                snprintf(output,
                         sizeof(output),
                         "L%f,%f %f,%f ",
                         points[0].X(), points[0].Y(),
                         points[1].X(), points[1].Y());
            } else if (curveType == difont::CurveType::Quadratic) {
                snprintf(output,
                         sizeof(output),"L%f,%f Q%f,%f %f,%f ",
                         points[0].X(), points[0].Y(),
                         points[1].X(), points[1].Y(),
                         points[2].X(), points[2].Y());

            } else if (curveType == difont::CurveType::Cubic) {
                snprintf(output,
                         sizeof(output),"L%f,%f C%f,%f %f,%f %f,%f ",
                         points[0].X(), points[0].Y(),
                         points[1].X(), points[1].Y(),
                         points[2].X(), points[2].Y(),
                         points[3].X(), points[3].Y());
            }
            return std::string(output);
        }

        difont::Point points[4];
        
        CurveType curveType;
    };
}

#endif