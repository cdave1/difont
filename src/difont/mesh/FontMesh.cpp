#include "FontMesh.h"

#include <stdio.h>
#include <assert.h>
#include <algorithm>

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
