#include <difont/difont.h>

#ifndef _FONT_PATH_H_
#define _FONT_PATH_H_

#include <vector>

namespace difont {
class Path {
    public:

        Path() {}

        Path(void* contour, char* tags, unsigned int n);

        void AddPath(const Path &path, const difont::Point &pen, bool clockwise=true);

        void SetLocation(difont::Point &a);

        void AddPoint(difont::Point &a);

        void AddLine(difont::Point &a, difont::Point &b);

        void AddQuadratic(difont::Point &a, difont::Point &b, difont::Point &c);

        void AddCubic(difont::Point &a, difont::Point &b, difont::Point &c, difont::Point &d);
    
        const std::vector<difont::Curve> GetCurves() const {
            return m_curves;
        };

        difont::Point GetLocation() const { return m_location; }

    private:
        std::vector<difont::Curve> m_curves;

        difont::Point m_location;

    };
}

#endif