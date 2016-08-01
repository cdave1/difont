#include <difont/difont.h>

#ifndef _FT_FONT_MESH_H_
#define _FT_FONT_MESH_H_

#include <vector>

namespace difont {
    class FontVertex {
    public:
        FontVertex() {
            position[0] = position[1] = position[2] = 0.0f;
            texCoord[0] = texCoord[1] = 0.0f;
        }

        void SetVertex3f(float x, float y, float z);

        void SetVertex2f(float x, float y);

        void SetTexCoord2f(float s, float t);

        float position[3];
        float texCoord[2];
    };


    class FontMesh {
    public:

        unsigned int textureId = 0;

        unsigned int primitive = 0;

        std::vector <difont::FontVertex> vertices;

    public:
        FontMesh() : primitive(0) {}

        FontMesh(unsigned int prim) : primitive(prim) {}

        void AddVertex(difont::FontVertex vertex);

        void SetTextureId(unsigned int texId);

        void SetPrimitive(unsigned int prim);
        
        unsigned long GetVertexCount() const;

    };
}

#endif