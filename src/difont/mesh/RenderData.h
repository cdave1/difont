#include <difont/difont.h>

#ifndef _FT_RENDER_DATA_H_
#define _FT_RENDER_DATA_H_

#include <vector>

namespace difont {
    class RenderData {
    public:

        RenderData() {}

        void AddGlyph(const difont::GlyphData &glyph);

        void AddMesh(const difont::FontMesh &mesh);

        int MeshCount() const;

        void Clear();

        const std::vector<difont::GlyphData> & GetGlyphData() const;

        const std::vector<difont::FontMesh> & GetMeshes() const;

    private:

        std::vector<difont::GlyphData> glyphs;

        std::vector<difont::FontMesh> meshes;

    };
}

#endif