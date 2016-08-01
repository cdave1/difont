#include "RenderData.h"

#include <stdio.h>
#include <assert.h>
#include <algorithm>

void difont::RenderData::AddMesh(const difont::FontMesh &mesh) {
    meshes.push_back(mesh);
}


void difont::RenderData::AddGlyph(const difont::GlyphData &glyphData) {
    glyphs.push_back(glyphData);
}


int difont::RenderData::MeshCount() const{
    return meshes.size();
}


void difont::RenderData::Clear() {
    meshes.clear();
    glyphs.clear();
}


const std::vector<difont::FontMesh> & difont::RenderData::GetMeshes() const {
    return meshes;
}


const std::vector<difont::GlyphData> & difont::RenderData::GetGlyphData() const {
    return glyphs;
}
