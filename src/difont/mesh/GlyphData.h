#include <difont/difont.h>

#ifndef _FONT_GLYPH_DATA_H_
#define _FONT_GLYPH_DATA_H_

#include <vector>

#include "Path.h"
#include "FontMesh.h"

namespace difont {
class GlyphData {
    public:

        GlyphData() {}
    
        const std::vector<difont::Path> GetPaths() const {
            return m_paths;
        }

        const std::vector<difont::FontMesh> & GetMeshes() const {
            return m_meshes;
        }

        void AddPath(difont::Path &path){
            m_paths.push_back(path);
        }

        void AddMesh(const difont::FontMesh &mesh) {
            m_meshes.push_back(mesh);
        }

    private:
        std::vector<difont::Path> m_paths;

        std::vector<difont::FontMesh> m_meshes;

    };
}

#endif