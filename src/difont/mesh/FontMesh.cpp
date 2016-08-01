#include "FontMesh.h"

#include <stdio.h>
#include <assert.h>
#include <algorithm>
#include <string>
#include <sstream>

///
/// difont::FontVertex
///

void difont::FontVertex::SetVertex3f(float x, float y, float z) {
    position[0] = x;
    position[1] = y;
    position[2] = z;
}


void difont::FontVertex::SetVertex2f(float x, float y) {
    position[0] = x;
    position[1] = y;
    position[2] = 0.0f;
}


void difont::FontVertex::SetTexCoord2f(float s, float t) {
    texCoord[0] = s;
    texCoord[1] = t;
}


///
/// difont::Mesh
///
void difont::FontMesh::SetPrimitive(unsigned int prim) {
    primitive = prim;
    vertices.clear();
}


void difont::FontMesh::AddVertex(difont::FontVertex vertex) {
    vertices.push_back(vertex);
}


void difont::FontMesh::SetTextureId(unsigned int texId) {
    textureId = texId;
}


unsigned long difont::FontMesh::GetVertexCount() const {
    return vertices.size();
}


std::string difont::FontMesh::ToSVG() const {
    std::stringstream svg;

    svg << "M" << vertices[0].position[0] << "," << vertices[0].position[1] << " ";

    for (int i = 0; i < vertices.size(); ++i) {
        difont::FontVertex v1 = vertices[i];
        difont::FontVertex v2 = vertices[(i + 1) % vertices.size()];
        svg << "L" << v1.position[0] << "," << v1.position[1] << " " << v2.position[0] << "," << v2.position[1] << " ";
    }

    return svg.str();
}