#include "Path.h"

#include <math.h>
#include <stdio.h>
#include <assert.h>

#include <difont/difont.h>
#include <difont/vectoriser/Contour.h>

difont::Path::Path(void* _contour, char* tags, unsigned int n) : m_curves() {
    FT_Vector *contour = static_cast<FT_Vector *>(_contour);
    difont::Point prev, cur(contour[(n - 1) % n]), next(contour[0]);
    difont::Point a, b = next - cur;
    double olddir, dir = atan2((next - cur).Y(), (next - cur).X());
    double angle = 0.0;

    // See http://freetype.sourceforge.net/freetype2/docs/glyphs/glyphs-6.html
    // for a full description of FreeType tags.
    for(unsigned int i = 0; i < n; i++)
    {
        prev = cur;
        cur = next;
        next = difont::Point(contour[(i + 1) % n]);
        olddir = dir;
        dir = atan2((next - cur).Y(), (next - cur).X());

        // Compute our path's new direction.
        double t = dir - olddir;
        if(t < -M_PI) t += 2 * M_PI;
        if(t > M_PI) t -= 2 * M_PI;
        angle += t;

        // Only process point tags we know.
        if(n < 2 || FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_On)
        {
            AddPoint(cur);
        }
        else if(FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_Conic)
        {
            difont::Point prev2 = prev, next2 = next;

            // Previous point is either the real previous point (an "on"
            // point), or the midpoint between the current one and the
            // previous "conic off" point.
            if(FT_CURVE_TAG(tags[(i - 1 + n) % n]) == FT_Curve_Tag_Conic)
            {
                prev2 = (cur + prev) * 0.5;
                AddPoint(prev2);
            }

            // Next point is either the real next point or the midpoint.
            if(FT_CURVE_TAG(tags[(i + 1) % n]) == FT_Curve_Tag_Conic)
            {
                next2 = (cur + next) * 0.5;
            }

            AddQuadratic(prev2, cur, next2);
        }
        else if(FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_Cubic
                && FT_CURVE_TAG(tags[(i + 1) % n]) == FT_Curve_Tag_Cubic)
        {
            difont::Point last(contour[(i + 2) % n]);
            AddCubic(prev, cur, next, last);
        }
    }

    // If final angle is positive (+2PI), it's an anti-clockwise contour,
    // otherwise (-2PI) it's clockwise.
    //clockwise = (angle < 0.0);
}


void difont::Path::AddContour(const difont::Contour *contour, const difont::Point &pen, float adjustmentFactor) {
    difont::Point offset(pen.X(), pen.Y());

    for (difont::Curve curve : contour->GetPath().GetCurves()) {
        if (curve.curveType == difont::CurveType::Point) {
            difont::Point a(offset.X() + curve.points[0].X() * adjustmentFactor,
                            offset.Y() + curve.points[0].Y() * -adjustmentFactor);
            AddPoint(a);
        } else if (curve.curveType == difont::CurveType::Line) {
            difont::Point a(offset.X() + curve.points[0].X() * adjustmentFactor,
                            offset.Y() + curve.points[0].Y() * -adjustmentFactor);
            difont::Point b(offset.X() + curve.points[1].X() * adjustmentFactor,
                            offset.Y() + curve.points[1].Y() * -adjustmentFactor);
            AddLine(a, b);
        } else if (curve.curveType == difont::CurveType::Quadratic) {
            difont::Point a(offset.X() + curve.points[0].X() * adjustmentFactor,
                            offset.Y() + curve.points[0].Y() * -adjustmentFactor);
            difont::Point b(offset.X() + curve.points[1].X() * adjustmentFactor,
                            offset.Y() + curve.points[1].Y() * -adjustmentFactor);
            difont::Point c(offset.X() + curve.points[2].X() * adjustmentFactor,
                            offset.Y() + curve.points[2].Y() * -adjustmentFactor);
            AddQuadratic(a, b, c);

        } else if (curve.curveType == difont::CurveType::Cubic) {
            difont::Point a(offset.X() + curve.points[0].X() * adjustmentFactor,
                            offset.Y() + curve.points[0].Y() * -adjustmentFactor);
            difont::Point b(offset.X() + curve.points[1].X() * adjustmentFactor,
                            offset.Y() + curve.points[1].Y() * -adjustmentFactor);
            difont::Point c(offset.X() + curve.points[2].X() * adjustmentFactor,
                            offset.Y() + curve.points[2].Y() * -adjustmentFactor);
            difont::Point d(offset.X() + curve.points[3].X() * adjustmentFactor,
                            offset.Y() + curve.points[3].Y() * -adjustmentFactor);
            AddCubic(a, b, c, d);
        }
    }

    difont::Point first(offset.X() + contour->GetPath().GetCurves()[0].points[0].X() * adjustmentFactor,
                        offset.Y() + contour->GetPath().GetCurves()[0].points[0].Y() * -adjustmentFactor);
    AddPoint(first);
}


void difont::Path::AddPoint(difont::Point &a) {
    difont::Curve point(a);
    m_curves.push_back(a);
}


void difont::Path::AddLine(difont::Point &a, difont::Point &b) {
    difont::Curve line(a, b);
    m_curves.push_back(line);
}


void difont::Path::AddQuadratic(difont::Point &a, difont::Point &b, difont::Point &c) {
    difont::Curve quadratic(a, b, c);
    m_curves.push_back(quadratic);
}


void difont::Path::AddCubic(difont::Point &a, difont::Point &b, difont::Point &c, difont::Point &d) {
    difont::Curve cubic(a, b, c, d);
    m_curves.push_back(cubic);
}
