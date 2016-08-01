#include <difont/difont.h>

#ifndef _FONT_GLYPH_DATA_H_
#define _FONT_GLYPH_DATA_H_

#include <vector>

#include "Path.h"

namespace difont {
class GlyphData {
    public:

        GlyphData() {}
    
        const std::vector<difont::Path> GetPaths() const {
            return m_paths;
        }

        void AddPath(difont::Path &path){
            m_paths.push_back(path);
        }

    private:
        std::vector<difont::Path> m_paths;

    };
}

#endif