LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := difont

DIFONT_SRC_PATH := $(LOCAL_PATH)/../../../src
DIFONT_LIB_PATH := $(LOCAL_PATH)/../../../lib
FREETYPE_PATH := $(DIFONT_LIB_PATH)/freetype2

LOCAL_CFLAGS := -DANDROID_NDK \
		-DFT2_BUILD_LIBRARY=1

LOCAL_CPPFLAGS := -DANDROID_NDK \
		-DFT2_BUILD_LIBRARY=1 \
		-include $(DIFONT_SRC_PATH)/difont/difont.h

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include_all \
			$(FREETYPE_PATH)/include \
            $(FREETYPE_PATH)/include/freetype \
            $(FREETYPE_PATH)/include/freetype/config \
            $(FREETYPE_PATH)/include/freetype/internal \
			$(DIFONT_SRC_PATH) \
			$(DIFONT_SRC_PATH)/difont \
			$(DIFONT_SRC_PATH)/difont/common \
			$(DIFONT_SRC_PATH)/difont/font \
			$(DIFONT_SRC_PATH)/difont/freetype \
			$(DIFONT_SRC_PATH)/difont/glyph \
			$(DIFONT_SRC_PATH)/difont/layout \
			$(DIFONT_SRC_PATH)/difont/opengl \
			$(DIFONT_SRC_PATH)/difont/vectoriser \
			$(DIFONT_LIB_PATH)/iGLU-1.0.0/include

DIFONT_SRC_FILES := \
	$(DIFONT_SRC_PATH)/difont/Buffer.cpp \
	$(DIFONT_SRC_PATH)/difont/common/Point.cpp \
	$(DIFONT_SRC_PATH)/difont/common/Size.cpp \
	$(DIFONT_SRC_PATH)/difont/font/Font.cpp \
	$(DIFONT_SRC_PATH)/difont/font/OutlineFont.cpp \
	$(DIFONT_SRC_PATH)/difont/font/PolygonFont.cpp \
	$(DIFONT_SRC_PATH)/difont/font/TextureFont.cpp \
	$(DIFONT_SRC_PATH)/difont/freetype/Charmap.cpp \
	$(DIFONT_SRC_PATH)/difont/freetype/Face.cpp \
	$(DIFONT_SRC_PATH)/difont/freetype/Library.cpp \
	$(DIFONT_SRC_PATH)/difont/freetype/GlyphContainer.cpp \
	$(DIFONT_SRC_PATH)/difont/glyph/Glyph.cpp \
	$(DIFONT_SRC_PATH)/difont/glyph/OutlineGlyph.cpp \
	$(DIFONT_SRC_PATH)/difont/glyph/PolygonGlyph.cpp \
	$(DIFONT_SRC_PATH)/difont/glyph/TextureGlyph.cpp \
	$(DIFONT_SRC_PATH)/difont/layout/Layout.cpp \
	$(DIFONT_SRC_PATH)/difont/layout/SimpleLayout.cpp \
	$(DIFONT_SRC_PATH)/difont/opengl/OpenGLInterface.android.cpp \
	$(DIFONT_SRC_PATH)/difont/vectoriser/Contour.cpp \
	$(DIFONT_SRC_PATH)/difont/vectoriser/FontMesh.cpp \
	$(DIFONT_SRC_PATH)/difont/vectoriser/Vectoriser.cpp

FREETYPE2_SRC_FILES := \
	$(FREETYPE_PATH)/src/autofit/autofit.c \
	$(FREETYPE_PATH)/src/base/basepic.c \
	$(FREETYPE_PATH)/src/base/ftapi.c \
	$(FREETYPE_PATH)/src/base/ftbase.c \
	$(FREETYPE_PATH)/src/base/ftbbox.c \
	$(FREETYPE_PATH)/src/base/ftbitmap.c \
	$(FREETYPE_PATH)/src/base/ftdbgmem.c \
	$(FREETYPE_PATH)/src/base/ftdebug.c \
	$(FREETYPE_PATH)/src/base/ftglyph.c \
	$(FREETYPE_PATH)/src/base/ftinit.c \
	$(FREETYPE_PATH)/src/base/ftpic.c \
	$(FREETYPE_PATH)/src/base/ftstroke.c \
	$(FREETYPE_PATH)/src/base/ftsynth.c \
	$(FREETYPE_PATH)/src/base/ftsystem.c \
	$(FREETYPE_PATH)/src/cff/cff.c \
	$(FREETYPE_PATH)/src/pshinter/pshinter.c \
	$(FREETYPE_PATH)/src/psnames/psnames.c \
	$(FREETYPE_PATH)/src/raster/raster.c \
	$(FREETYPE_PATH)/src/sfnt/sfnt.c \
	$(FREETYPE_PATH)/src/smooth/smooth.c \
	$(FREETYPE_PATH)/src/truetype/truetype.c

IGLU_SRC_FILES := \
	$(DIFONT_LIB_PATH)/iGLU-1.0.0/libtess/dict.c \
	$(DIFONT_LIB_PATH)/iGLU-1.0.0/libtess/geom.c \
	$(DIFONT_LIB_PATH)/iGLU-1.0.0/libtess/memalloc.c \
	$(DIFONT_LIB_PATH)/iGLU-1.0.0/libtess/mesh.c \
	$(DIFONT_LIB_PATH)/iGLU-1.0.0/libtess/normal.c \
	$(DIFONT_LIB_PATH)/iGLU-1.0.0/libtess/priorityq.c \
	$(DIFONT_LIB_PATH)/iGLU-1.0.0/libtess/render.c \
	$(DIFONT_LIB_PATH)/iGLU-1.0.0/libtess/sweep.c \
	$(DIFONT_LIB_PATH)/iGLU-1.0.0/libtess/tess.c \
	$(DIFONT_LIB_PATH)/iGLU-1.0.0/libtess/tessmono.c \
	$(DIFONT_LIB_PATH)/iGLU-1.0.0/libutil/error.c \
	$(DIFONT_LIB_PATH)/iGLU-1.0.0/libutil/glue.c \
	$(DIFONT_LIB_PATH)/iGLU-1.0.0/libutil/project.c \
	$(DIFONT_LIB_PATH)/iGLU-1.0.0/libutil/registry.c

LOCAL_SRC_FILES := $(FREETYPE2_SRC_FILES) $(IGLU_SRC_FILES) $(DIFONT_SRC_FILES) 

LOCAL_LDLIBS := -lGLESv3 -llog -ldl -lstdc++

include $(BUILD_SHARED_LIBRARY)