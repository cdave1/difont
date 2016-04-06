#ifndef _DIFONT_H_
#define _DIFONT_H_

#include <stdlib.h>

/// Freetype files
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "freetype/ftoutln.h"

#ifdef __cplusplus
namespace difont {
    typedef enum {
        RENDER_FRONT = 0x0001,
        RENDER_BACK  = 0x0002,
        RENDER_SIDE  = 0x0004,
        RENDER_ALL   = 0xffff
    } RenderMode;

    typedef enum {
        ALIGN_LEFT    = 0,
        ALIGN_CENTER  = 1,
        ALIGN_RIGHT   = 2,
        ALIGN_JUSTIFY = 3
    } TextAlignment;
}
#endif

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif

#include "common/Point.h"
#include "common/BBox.h"
#include "Buffer.h"

#include "Glyph.h"

#include "BitmapGlyph.h"
#include "BufferGlyph.h"
//#include "ExtrdGlyph.h"
#include "OutlineGlyph.h"
#include "PixmapGlyph.h"
#include "PolyGlyph.h"
#include "TextureGlyph.h"

#include "Font.h"
#include "BitmapFont.h"
#include "BufferFont.h"
//#include "ExtrdFont.h"
#include "OutlineFont.h"
#include "PixmapFont.h"
#include "PolygonFont.h"
#include "TextureFont.h"

#include "Layout.h"
#include "SimpleLayout.h"

#include "mesh/FontMesh.h"

#endif