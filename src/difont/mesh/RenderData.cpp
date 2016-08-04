#include "RenderData.h"

#include <stdio.h>
#include <assert.h>
#include <algorithm>

void difont::RenderData::AddGlyph(const difont::GlyphData &glyphData) {
    glyphs.push_back(glyphData);
}


void difont::RenderData::Clear() {
    glyphs.clear();
}


const std::vector<difont::GlyphData> & difont::RenderData::GetGlyphData() const {
    return glyphs;
}
