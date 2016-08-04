#include <difont/difont.h>

#ifndef _FT_RENDER_DATA_H_
#define _FT_RENDER_DATA_H_

#include <vector>

namespace difont {
    class RenderData {
    public:

        RenderData() {}

        void AddGlyph(const difont::GlyphData &glyph);

        void Clear();

        const std::vector<difont::GlyphData> & GetGlyphData() const;

    private:

        std::vector<difont::GlyphData> glyphs;

    };
}

#endif