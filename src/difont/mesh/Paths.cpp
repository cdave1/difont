#include "Paths.h"

#include <stdio.h>
#include <assert.h>


void difont::Path::Begin() {
    curveCount = 0;
}


void difont::Path::AddQuadratic(CurvePoint a, CurvePoint b, CurvePoint c) {

}


void difont::Path::AddCubic(CurvePoint a, CurvePoint b, CurvePoint c, CurvePoint d) {
    difont::Curve curve(a, b, c, d);
    
}


void difont::Path::End() {
}


int difont::Path::CurveCount() {
    return curveCount;
}


difont::Curve * difont::Path::GetCurves() {
    return curves;
}
