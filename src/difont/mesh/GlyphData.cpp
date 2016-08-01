#include "GlyphData.h"



/*
void difont::Path::DEBUG_toSVG(const difont::Point &pen, bool clockwise) const {
    difont::Point offset(pen.X(), pen.Y());

    printf("M%f,%f ", offset.X() + m_curves[0].points[0].X() / 64.0, offset.Y() + m_curves[0].points[0].Y() / 64.0);

    for (int i = 0; i < m_curves.size(); ++i) {
        difont::Curve curve = m_curves[i];

        if (curve.curveType == difont::CurveType::Point) {
            printf("L%f,%f ",
                   offset.X() + curve.points[0].X() / 64.0, offset.Y() + curve.points[0].Y() / 64.0);
        } else if (curve.curveType == difont::CurveType::Line) {
            printf("L%f,%f %f,%f ",
                   offset.X() + curve.points[0].X() / 64.0, offset.Y() + curve.points[0].Y() / 64.0,
                   offset.X() + curve.points[1].X() / 64.0, offset.Y() + curve.points[1].Y() / 64.0);
        } else if (curve.curveType == difont::CurveType::Quadratic) {
            printf("L%f,%f Q%f,%f %f,%f ",
                   offset.X() + curve.points[0].X() / 64.0, offset.Y() + curve.points[0].Y() / 64.0,
                   offset.X() + curve.points[1].X() / 64.0, offset.Y() + curve.points[1].Y() / 64.0,
                   offset.X() + curve.points[2].X() / 64.0, offset.Y() + curve.points[2].Y() / 64.0);

        } else if (curve.curveType == difont::CurveType::Cubic) {
            printf("L%f,%f C%f,%f %f,%f %f,%f ",
                   offset.X() + curve.points[0].X() / 64.0, offset.Y() + curve.points[0].Y() / 64.0,
                   offset.X() + curve.points[1].X() / 64.0, offset.Y() + curve.points[1].Y() / 64.0,
                   offset.X() + curve.points[2].X() / 64.0, offset.Y() + curve.points[2].Y() / 64.0,
                   offset.X() + curve.points[3].X() / 64.0, offset.Y() + curve.points[3].Y() / 64.0);
        }
    }

    if (!clockwise) {
        printf("L%f,%f ", offset.X() + m_curves[0].points[0].X() / 64.0, offset.Y() + m_curves[0].points[0].Y() / 64.0);
    }
}*/
