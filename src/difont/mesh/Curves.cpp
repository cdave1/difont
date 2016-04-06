#include "FontCurves.h"

#include <stdio.h>
#include <assert.h>

static difont::Curve curves[65536];
static int curveCount = 0;


void difont::CurveList::Begin() {
    curveCount = 0;
}


void difont::CurveList::AddQuadratic(CurvePoint a, CurvePoint b, CurvePoint c) {
    curveCount++;
}


void difont::CurveList::AddCubic(CurvePoint a, CurvePoint b, CurvePoint c, CurvePoint d) {
    curveCount++;
}


void difont::CurveList::End() {
}


int difont::CurveList::CurveCount() {
    return curveCount;
}


difont::Curve * difont::CurveList::GetCurves() {
    return curves;
}
