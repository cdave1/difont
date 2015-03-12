/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 * Copyright (c) 2008 Sam Hocevar <sam@zoy.org>
 * Copyright (c) 2008 Sean Morrison <learner@brlcad.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <difont/difont.h>

#ifndef _DIFONT_FONT_H_
#define _DIFONT_FONT_H_

/**
 * Font is the public interface for the FTGL library.
 *
 * Specific font classes are derived from this class. It uses the helper
 * classes Face and difont::Size to access the Freetype library. This class
 * is abstract and deriving classes must implement the protected
 * <code>MakeGlyph</code> function to create glyphs of the
 * appropriate type.
 *
 * It is good practice after using these functions to test the error
 * code returned. <code>FT_Error Error()</code>. Check the freetype file
 * fterrdef.h for error definitions.
 *
 * @see     Face
 * @see     difont::Size
 */
namespace difont {
    class FontImpl;

    class Font {
    protected:
        /**
         * Open and read a font file. Sets Error flag.
         *
         * @param fontFilePath  font file path.
         */
        Font(char const *fontFilePath);

        /**
         * Open and read a font from a buffer in memory. Sets Error flag.
         * The buffer is owned by the client and is NOT copied by FTGL. The
         * pointer must be valid while using FTGL.
         *
         * @param pBufferBytes  the in-memory buffer
         * @param bufferSizeInBytes  the length of the buffer in bytes
         */
        Font(const unsigned char *pBufferBytes, size_t bufferSizeInBytes);

    private:
        /* Allow our internal subclasses to access the private constructor */
        friend class BitmapFont;
        friend class BufferFont;
        friend class ExtrudeFont;
        friend class OutlineFont;
        friend class PixmapFont;
        friend class PolygonFont;
        friend class TextureFont;

        /**
         * Internal FTGL Font constructor. For private use only.
         *
         * @param pImpl  Internal implementation object. Will be destroyed
         *               upon Font deletion.
         */
        Font(FontImpl *pImpl);

    public:
        virtual ~Font();

        /**
         * Attach auxilliary file to font e.g font metrics.
         *
         * Note: not all font formats implement this function.
         *
         * @param fontFilePath  auxilliary font file path.
         * @return          <code>true</code> if file has been attached
         *                  successfully.
         */
        virtual bool Attach(const char* fontFilePath);

        /**
         * Attach auxilliary data to font e.g font metrics, from memory.
         *
         * Note: not all font formats implement this function.
         *
         * @param pBufferBytes  the in-memory buffer.
         * @param bufferSizeInBytes  the length of the buffer in bytes.
         * @return          <code>true</code> if file has been attached
         *                  successfully.
         */
        virtual bool Attach(const unsigned char *pBufferBytes,
                            size_t bufferSizeInBytes);

        /**
         * Set the glyph loading flags. By default, fonts use the most
         * sensible flags when loading a font's glyph using FT_Load_Glyph().
         * This function allows to override the default flags.
         *
         * @param flags  The glyph loading flags.
         */
        virtual void GlyphLoadFlags(FT_Int flags);

        /**
         * Set the character map for the face.
         *
         * @param encoding      Freetype enumerate for char map code.
         * @return              <code>true</code> if charmap was valid and
         *                      set correctly.
         */
        virtual bool CharMap(FT_Encoding encoding);

        /**
         * Get the number of character maps in this face.
         *
         * @return character map count.
         */
        virtual unsigned int CharMapCount() const;

        /**
         * Get a list of character maps in this face.
         *
         * @return pointer to the first encoding.
         */
        virtual FT_Encoding* CharMapList();

        /**
         * Set the char size for the current face.
         *
         * @param size      the face size in points (1/72 inch)
         * @param res       the resolution of the target device.
         * @return          <code>true</code> if size was set correctly
         */
        virtual bool FaceSize(const unsigned int size,
                              const unsigned int res = 72);

        /**
         * Get the current face size in points (1/72 inch).
         *
         * @return face size
         */
        virtual unsigned int FaceSize() const;

        /**
         * Set the extrusion distance for the font. Only implemented by
         * ExtrudeFont
         *
         * @param depth  The extrusion distance.
         */
        virtual void Depth(float depth);

        /**
         * Set the outset distance for the font. Only implemented by
         * OutlineFont, PolygonFont and ExtrudeFont
         *
         * @param outset  The outset distance.
         */
        virtual void Outset(float outset);

        /**
         * Set the front and back outset distances for the font. Only
         * implemented by ExtrudeFont
         *
         * @param front  The front outset distance.
         * @param back   The back outset distance.
         */
        virtual void Outset(float front, float back);

        /**
         * Enable or disable the use of Display Lists inside FTGL
         *
         * @param  useList <code>true</code> turns ON display lists.
         *                 <code>false</code> turns OFF display lists.
         */
        virtual void UseDisplayList(bool useList);

        /**
         * Get the global ascender height for the face.
         *
         * @return  Ascender height
         */
        virtual float Ascender() const;

        /**
         * Gets the global descender height for the face.
         *
         * @return  Descender height
         */
        virtual float Descender() const;

        /**
         * Gets the line spacing for the font.
         *
         * @return  Line height
         */
        virtual float LineHeight() const;

        /**
         * Get the bounding box for a string.
         *
         * @param string  A char buffer.
         * @param len  The length of the string. If < 0 then all characters
         *             will be checked until a null character is encountered
         *             (optional).
         * @param position  The pen position of the first character (optional).
         * @param spacing  A displacement vector to add after each character
         *                 has been checked (optional).
         * @return  The corresponding bounding box.
         */
        virtual difont::BBox BBox(const char *string, const int len = -1,
                                  difont::Point position = difont::Point(),
                                  difont::Point spacing = difont::Point());

        /**
         * Get the bounding box for a string (deprecated).
         *
         * @param string  A char buffer.
         * @param llx  Lower left near x coordinate.
         * @param lly  Lower left near y coordinate.
         * @param llz  Lower left near z coordinate.
         * @param urx  Upper right far x coordinate.
         * @param ury  Upper right far y coordinate.
         * @param urz  Upper right far z coordinate.
         */
        void BBox(const char* string, float& llx, float& lly, float& llz,
                  float& urx, float& ury, float& urz)
        {
            difont::BBox b = BBox(string);
            llx = b.Lower().Xf(); lly = b.Lower().Yf(); llz = b.Lower().Zf();
            urx = b.Upper().Xf(); ury = b.Upper().Yf(); urz = b.Upper().Zf();
        }

        /**
         * Get the bounding box for a string.
         *
         * @param string  A wchar_t buffer.
         * @param len  The length of the string. If < 0 then all characters
         *             will be checked until a null character is encountered
         *             (optional).
         * @param position  The pen position of the first character (optional).
         * @param spacing  A displacement vector to add after each character
         *                 has been checked (optional).
         * @return  The corresponding bounding box.
         */
        virtual difont::BBox BBox(const wchar_t *string, const int len = -1,
                                  difont::Point position = difont::Point(),
                                  difont::Point spacing = difont::Point());

        /**
         * Get the bounding box for a string (deprecated).
         *
         * @param string  A wchar_t buffer.
         * @param llx  Lower left near x coordinate.
         * @param lly  Lower left near y coordinate.
         * @param llz  Lower left near z coordinate.
         * @param urx  Upper right far x coordinate.
         * @param ury  Upper right far y coordinate.
         * @param urz  Upper right far z coordinate.
         */
        void BBox(const wchar_t* string, float& llx, float& lly, float& llz,
                  float& urx, float& ury, float& urz)
        {
            difont::BBox b = BBox(string);
            llx = b.Lower().Xf(); lly = b.Lower().Yf(); llz = b.Lower().Zf();
            urx = b.Upper().Xf(); ury = b.Upper().Yf(); urz = b.Upper().Zf();
        }

        /**
         * Get the advance for a string.
         *
         * @param string  'C' style string to be checked.
         * @param len  The length of the string. If < 0 then all characters
         *             will be checked until a null character is encountered
         *             (optional).
         * @param spacing  A displacement vector to add after each character
         *                 has been checked (optional).
         * @return  The string's advance width.
         */
        virtual float Advance(const char* string, const int len = -1,
                              difont::Point spacing = difont::Point());

        /**
         * Get the advance for a string.
         *
         * @param string  A wchar_t string
         * @param len  The length of the string. If < 0 then all characters
         *             will be checked until a null character is encountered
         *             (optional).
         * @param spacing  A displacement vector to add after each character
         *                 has been checked (optional).
         * @return  The string's advance width.
         */
        virtual float Advance(const wchar_t* string, const int len = -1,
                              difont::Point spacing = difont::Point());

        /**
         * Render a string of characters.
         *
         * @param string  'C' style string to be output.
         * @param len  The length of the string. If < 0 then all characters
         *             will be displayed until a null character is encountered
         *             (optional).
         * @param position  The pen position of the first character (optional).
         * @param spacing  A displacement vector to add after each character
         *                 has been displayed (optional).
         * @param renderMode  Render mode to use for display (optional).
         * @return  The new pen position after the last character was output.
         */
        virtual difont::Point Render(const char* string, const int len = -1,
                                     difont::Point position = difont::Point(),
                                     difont::Point spacing = difont::Point(),
                                     int renderMode = FTGL::RENDER_ALL);

        /**
         * Render a string of characters
         *
         * @param string    wchar_t string to be output.
         * @param len  The length of the string. If < 0 then all characters
         *             will be displayed until a null character is encountered
         *             (optional).
         * @param position  The pen position of the first character (optional).
         * @param spacing  A displacement vector to add after each character
         *                 has been displayed (optional).
         * @param renderMode  Render mode to use for display (optional).
         * @return  The new pen position after the last character was output.
         */
        virtual difont::Point Render(const wchar_t *string, const int len = -1,
                                     difont::Point position = difont::Point(),
                                     difont::Point spacing = difont::Point(),
                                     int renderMode = FTGL::RENDER_ALL);


        virtual void PreRender();


        virtual void PostRender();


        /**
         * Queries the Font for errors.
         *
         * @return  The current error code.
         */
        virtual FT_Error Error() const;

    protected:
        /* Allow impl to access MakeGlyph */
        friend class FontImpl;

        /**
         * Construct a glyph of the correct type.
         *
         * Clients must override the function and return their specialised
         * Glyph.
         *
         * @param slot  A FreeType glyph slot.
         * @return  An FT****Glyph or <code>null</code> on failure.
         */
        virtual Glyph* MakeGlyph(FT_GlyphSlot slot) = 0;
        
    private:
        /**
         * Internal FTGL Font implementation object. For private use only.
         */
        FontImpl *impl;
    };
}

#endif  //  __Font__
