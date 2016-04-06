#ifndef _FONT_PATHS_H_
#define _FONT_PATHS_H_

#include "mesh/Curves.h"
#include <vector>

namespace difont {
class Path {
    private:
        std::vector<difont::Curve *> m_curves;

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