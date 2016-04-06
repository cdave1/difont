#ifndef _FT_FONT_MESH_H_
#define _FT_FONT_MESH_H_

#define DIFONT_MESH_MAX_VERTICES 512

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
        FontMesh();

        void AddVertex(difont::FontVertex vertex);

        void SetTextureId(unsigned int texId);

        void SetPrimitive(unsigned int prim);

        unsigned int GetVertexCount() const;

        FontVertex vertices[DIFONT_MESH_MAX_VERTICES];

        unsigned int currIndex;

        unsigned int textureId;

        unsigned int primitive;
    };


    class FontMeshSet {
    public:

        static void Begin();

        static void AddMesh(unsigned int prim);

        static void SetTextureId(unsigned int texId);

        static void AddVertex(difont::FontVertex vertex);

        static void End();

        static int MeshCount();

        static difont::FontMesh * GetMeshes();

    };
}

#endif