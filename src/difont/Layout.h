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

#ifndef _DIFONT_Layout__
#define _DIFONT_Layout__

/**
 * Layout is the interface for layout managers that render text.
 *
 * Specific layout manager classes are derived from this class. This class
 * is abstract and deriving classes must implement the protected
 * <code>Render</code> methods to render formatted text and
 * <code>BBox</code> methods to determine the bounding box of output text.
 *
 * @see     Font
 * @see     difont::BBox
 */
namespace difont {
    class LayoutImpl;

    class Layout {
    protected:
        Layout();

    private:
        /**
         * Internal FTGL Layout constructor. For private use only.
         *
         * @param pImpl  Internal implementation object. Will be destroyed
         *               upon Layout deletion.
         */
        Layout(LayoutImpl *pImpl);

        /* Allow our internal subclasses to access the private constructor */
        friend class SimpleLayout;

    public:
        /**
         * Destructor
         */
        virtual ~Layout();

        /**
         * Get the bounding box for a formatted string.
         *
         * @param string  A char string.
         * @param len  The length of the string. If < 0 then all characters
         *             will be checked until a null character is encountered
         *             (optional).
         * @param position  The pen position of the first character (optional).
         * @return  The corresponding bounding box.
         */
        virtual difont::BBox BBox(const char* string, const int len = -1,
                                  difont::Point position = difont::Point()) = 0;

        /**
         * Get the bounding box for a formatted string.
         *
         * @param string  A wchar_t string.
         * @param len  The length of the string. If < 0 then all characters
         *             will be checked until a null character is encountered
         *             (optional).
         * @param position  The pen position of the first character (optional).
         * @return  The corresponding bounding box.
         */
        virtual difont::BBox BBox(const wchar_t* string, const int len = -1,
                                  difont::Point position = difont::Point()) = 0;

        /**
         * Render a string of characters.
         *
         * @param string    'C' style string to be output.
         * @param len  The length of the string. If < 0 then all characters
         *             will be displayed until a null character is encountered
         *             (optional).
         * @param position  The pen position of the first character (optional).
         * @param renderMode  Render mode to display (optional)
         */
        virtual void Render(const char *string, const int len = -1,
                            difont::Point position = difont::Point(),
                            int renderMode = FTGL::RENDER_ALL) = 0;

        /**
         * Render a string of characters.
         *
         * @param string    wchar_t string to be output.
         * @param len  The length of the string. If < 0 then all characters
         *             will be displayed until a null character is encountered
         *             (optional).
         * @param position  The pen position of the first character (optional).
         * @param renderMode  Render mode to display (optional)
         */
        virtual void Render(const wchar_t *string, const int len = -1,
                            difont::Point position = difont::Point(),
                            int renderMode = FTGL::RENDER_ALL) = 0;

        /**
         * Queries the Layout for errors.
         *
         * @return  The current error code.
         */
        virtual FT_Error Error() const;

    private:
        /**
         * Internal FTGL Layout implementation object. For private use only.
         */
        LayoutImpl *impl;
    };
}

#endif

