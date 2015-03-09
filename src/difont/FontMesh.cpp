#include "FontMesh.h"

#include <stdio.h>
#include <assert.h>
#include <algorithm>

static difont::FontMesh meshes[65536];
static int meshCount = 0;


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
difont::FontMesh::FontMesh() : currIndex(0), textureId(0), primitive(0) {
}


void difont::FontMesh::SetPrimitive(unsigned int prim) {
    primitive = prim;
    currIndex = 0;
}


void difont::FontMesh::AddVertex(difont::FontVertex vertex) {
    vertices[currIndex] = vertex;
    currIndex++;
}


void difont::FontMesh::SetTextureId(unsigned int texId) {
    textureId = texId;
}


unsigned int difont::FontMesh::GetVertexCount() const {
    return currIndex;
}


///
/// difont::FontMesh
///

void difont::FontMeshSet::Begin() {
    meshCount = 0;
}


void difont::FontMeshSet::AddMesh(unsigned int prim) {
    meshes[meshCount].SetPrimitive(prim);
    meshes[meshCount].currIndex = 0;
    meshCount++;
}


void difont::FontMeshSet::SetTextureId(unsigned int texId) {
    int meshIndex = std::max(0, meshCount - 1);
    meshes[meshIndex].SetTextureId(texId);
}


void difont::FontMeshSet::AddVertex(difont::FontVertex vertex) {
    int meshIndex = std::max(0, meshCount - 1);
    meshes[meshIndex].AddVertex(vertex);
}


void difont::FontMeshSet::End() {
}


int difont::FontMeshSet::MeshCount() {
    return meshCount;
}


difont::FontMesh * difont::FontMeshSet::GetMeshes() {
    return meshes;
}
