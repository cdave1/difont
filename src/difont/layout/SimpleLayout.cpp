/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 * Copyright (c) 2008 Sam Hocevar <sam@zoy.org>
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

#include <ctype.h>
#include <wctype.h>

#include "Internals.h"
#include "Unicode.h"

#include "GlyphContainer.h"
#include "SimpleLayoutImpl.h"


//
//  SimpleLayout
//
using namespace difont;

SimpleLayout::SimpleLayout() :
Layout(new SimpleLayoutImpl())
{}


SimpleLayout::~SimpleLayout()
{}


difont::BBox SimpleLayout::BBox(const char *string, const int len, difont::Point pos)
{
    assert(this);
    assert(impl);
    assert(strlen(string) > 0);
    return dynamic_cast<SimpleLayoutImpl*>(impl)->BBox(string, len, pos);
}


difont::BBox SimpleLayout::BBox(const wchar_t *string, const int len, difont::Point pos)
{
    return dynamic_cast<SimpleLayoutImpl*>(impl)->BBox(string, len, pos);
}


void SimpleLayout::Render(const char *string, const int len, difont::Point pos,
                            int renderMode)
{
    return dynamic_cast<SimpleLayoutImpl*>(impl)->Render(string, len, pos,
                                                           renderMode);
}


void SimpleLayout::Render(const wchar_t* string, const int len, difont::Point pos,
                            int renderMode)
{
    return dynamic_cast<SimpleLayoutImpl*>(impl)->Render(string, len, pos,
                                                           renderMode);
}


void SimpleLayout::SetFont(Font *fontInit)
{
    dynamic_cast<SimpleLayoutImpl*>(impl)->currentFont = fontInit;
}


Font *SimpleLayout::GetFont()
{
    return dynamic_cast<SimpleLayoutImpl*>(impl)->currentFont;
}


void SimpleLayout::SetLineLength(const float LineLength)
{
    dynamic_cast<SimpleLayoutImpl*>(impl)->lineLength = LineLength;
}


float SimpleLayout::GetLineLength() const
{
    return dynamic_cast<SimpleLayoutImpl*>(impl)->lineLength;
}


void SimpleLayout::SetAlignment(const FTGL::TextAlignment Alignment)
{
    dynamic_cast<SimpleLayoutImpl*>(impl)->alignment = Alignment;
}


FTGL::TextAlignment SimpleLayout::GetAlignment() const
{
    return dynamic_cast<SimpleLayoutImpl*>(impl)->alignment;
}


void SimpleLayout::SetLineSpacing(const float LineSpacing)
{
    dynamic_cast<SimpleLayoutImpl*>(impl)->lineSpacing = LineSpacing;
}


float SimpleLayout::GetLineSpacing() const
{
    return dynamic_cast<SimpleLayoutImpl*>(impl)->lineSpacing;
}


//
//  SimpleLayoutImpl
//


SimpleLayoutImpl::SimpleLayoutImpl()
{
    currentFont = NULL;
    lineLength = 100.0f;
    alignment = FTGL::ALIGN_LEFT;
    lineSpacing = 1.0f;
	stringCacheCount = 0;
    layoutStringBuffer = 0;
}


template <typename T>
inline difont::BBox SimpleLayoutImpl::BBoxI(const T* string, const int len,
                                        difont::Point position)
{
    difont::BBox tmp;
	
    WrapText(string, len, position, 0, &tmp);
	
    return tmp;
}


difont::BBox SimpleLayoutImpl::BBox(const char *string, const int len,
                                difont::Point position)
{
    return BBoxI(string, len, position);
}


difont::BBox SimpleLayoutImpl::BBox(const wchar_t *string, const int len,
                                difont::Point position)
{
    return BBoxI(string, len, position);
}


template <typename T>
inline void SimpleLayoutImpl::RenderI(const T *string, const int len,
                                        difont::Point position, int renderMode)
{
    pen = difont::Point(0.0f, 0.0f);
    WrapText(string, len, position, renderMode, NULL);
}


void SimpleLayoutImpl::Render(const char *string, const int len,
                                difont::Point position, int renderMode)
{
    RenderI(string, len, position, renderMode);
}


void SimpleLayoutImpl::Render(const wchar_t* string, const int len,
                                difont::Point position, int renderMode)
{
    RenderI(string, len, position, renderMode);
}


template <typename T>
inline void SimpleLayoutImpl::WrapTextI(const T *buf, const int len,
                                          unsigned int stringLen, difont::Point position,
                                          int renderMode, difont::BBox *bounds)
{
    FTUnicodeStringItr<T> breakItr(buf);          // points to the last break character
    FTUnicodeStringItr<T> lineStart(buf);         // points to the line start
    float nextStart = 0.0;     // total width of the current line
    float breakWidth = 0.0;    // width of the line up to the last word break
    float currentWidth = 0.0;  // width of all characters on the current line
    float prevWidth;           // width of all characters but the current glyph
    float wordLength = 0.0;    // length of the block since the last break char
    int charCount = 0;         // number of characters so far on the line
    int breakCharCount = 0;    // number of characters before the breakItr
    float glyphWidth, advance;
    difont::BBox glyphBounds;

    // Reset the pen position
    pen.Y(0);

    // If we have bounds mark them invalid
    if(bounds)
    {
        bounds->Invalidate();
    }

    // Scan the input for all characters that need output
    FTUnicodeStringItr<T> prevItr(buf);
    for (FTUnicodeStringItr<T> itr(buf); *itr; prevItr = itr++, charCount++)
    {
        // Find the width of the current glyph
        glyphBounds = currentFont->BBox(itr.getBufferFromHere(), 1);
        glyphWidth = glyphBounds.Upper().Xf() - glyphBounds.Lower().Xf();

        advance = currentFont->Advance(itr.getBufferFromHere(), 1);
        prevWidth = currentWidth;
        // Compute the width of all glyphs up to the end of buf[i]
        currentWidth = nextStart + glyphWidth;
        // Compute the position of the next glyph
        nextStart += advance;

        // See if the current character is a space, a break or a regular character
        if((currentWidth > lineLength) || (*itr == '\n'))
        {
            // A non whitespace character has exceeded the line length.  Or a
            // newline character has forced a line break.  Output the last
            // line and start a new line after the break character.
            // If we have not yet found a break, break on the last character
            if(breakItr == lineStart || (*itr == '\n'))
            {
                // Break on the previous character
                breakItr = prevItr;
                breakCharCount = charCount - 1;
                breakWidth = prevWidth;
                // None of the previous words will be carried to the next line
                wordLength = 0;
                // If the current character is a newline discard its advance
                if(*itr == '\n') advance = 0;
            }

            float remainingWidth = lineLength - breakWidth;

            // Render the current substring
            FTUnicodeStringItr<T> breakChar = breakItr;
            // move past the break character and don't count it on the next line either
            ++breakChar; --charCount;
            // If the break character is a newline do not render it
            if(*breakChar == '\n')
            {
                ++breakChar; --charCount;
            }

            OutputWrapped(lineStart.getBufferFromHere(), breakCharCount,
                          //breakItr.getBufferFromHere() - lineStart.getBufferFromHere(),
                          position, renderMode, remainingWidth, bounds);

            // Store the start of the next line
            lineStart = breakChar;
            // TODO: Is Height() the right value here?
            pen -= difont::Point(0, currentFont->LineHeight() * lineSpacing);
            // The current width is the width since the last break
            nextStart = wordLength + advance;
            wordLength += advance;
            currentWidth = wordLength + advance;
            // Reset the safe break for the next line
            breakItr = lineStart;
            charCount -= breakCharCount;
        }
        else if(iswspace(*itr))
        {
            // This is the last word break position
            wordLength = 0;
            breakItr = itr;
            breakCharCount = charCount;

            // Check to see if this is the first whitespace character in a run
            if(buf == itr.getBufferFromHere() || !iswspace(*prevItr))
            {
                // Record the width of the start of the block
                breakWidth = currentWidth;
            }
        }
        else
        {
            wordLength += advance;
        }
    }

    float remainingWidth = lineLength - currentWidth;
    // Render any remaining text on the last line
    // Disable justification for the last row
    if(alignment == FTGL::ALIGN_JUSTIFY)
    {
        alignment = FTGL::ALIGN_LEFT;
        OutputWrapped(lineStart.getBufferFromHere(), -1, position, renderMode,
                      remainingWidth, bounds);
        alignment = FTGL::ALIGN_JUSTIFY;
    }
    else
    {
        OutputWrapped(lineStart.getBufferFromHere(), -1, position, renderMode,
                      remainingWidth, bounds);
    }
}


void SimpleLayoutImpl::WrapText(const char *buf, const int len,
                                  difont::Point position, int renderMode,
                                  difont::BBox *bounds)
{
    WrapTextI(buf, len, strlen(buf), position, renderMode, bounds);
}


void SimpleLayoutImpl::WrapText(const wchar_t* buf, const int len,
                                  difont::Point position, int renderMode,
                                  difont::BBox *bounds)
{
    WrapTextI(buf, len, wcslen(buf), position, renderMode, bounds);
}


template <typename T>
inline void SimpleLayoutImpl::OutputWrappedI(const T *buf, const int len,
                                               difont::Point position, int renderMode,
                                               const float remaining,
                                               difont::BBox *bounds)
{
    float distributeWidth = 0.0;
    // Align the text according as specified by Alignment
    switch (alignment)
    {
        case FTGL::ALIGN_LEFT:
            pen.X(0);
            break;
        case FTGL::ALIGN_CENTER:
            pen.X(remaining / 2);
            break;
        case FTGL::ALIGN_RIGHT:
            pen.X(remaining);
            break;
        case FTGL::ALIGN_JUSTIFY:
            pen.X(0);
            distributeWidth = remaining;
            break;
    }
	
    // If we have bounds expand them by the line's bounds, otherwise render
    // the line.
    if(bounds)
    {
        difont::BBox temp = currentFont->BBox(buf, len);
		
        // Add the extra space to the upper x dimension
        temp = difont::BBox(temp.Lower() + pen,
                      temp.Upper() + pen + difont::Point(distributeWidth, 0));
		
        // See if this is the first area to be added to the bounds
        if(bounds->IsValid())
        {
            *bounds |= temp;
        }
        else
        {
            *bounds = temp;
        }
    }
    else
    {
        RenderSpace(buf, len, position, renderMode, distributeWidth);
    }
}


void SimpleLayoutImpl::OutputWrapped(const char *buf, const int len,
                                       difont::Point position, int renderMode,
                                       const float remaining, difont::BBox *bounds)
{
    OutputWrappedI(buf, len, position, renderMode, remaining, bounds);
}


void SimpleLayoutImpl::OutputWrapped(const wchar_t *buf, const int len,
                                       difont::Point position, int renderMode,
                                       const float remaining, difont::BBox *bounds)
{
    OutputWrappedI(buf, len, position, renderMode, remaining, bounds);
}


template <typename T>
inline void SimpleLayoutImpl::RenderSpaceI(const T *string, const int len,
                                             difont::Point position, int renderMode,
                                             const float extraSpace)
{
    float space = 0.0;
	
    // If there is space to distribute, count the number of spaces
    if(extraSpace > 0.0)
    {
        int numSpaces = 0;
		
        // Count the number of space blocks in the input
        FTUnicodeStringItr<T> prevItr(string), itr(string);
        for(int i = 0; ((len < 0) && *itr) || ((len >= 0) && (i <= len));
            ++i, prevItr = itr++)
        {
            // If this is the end of a space block, increment the counter
            if((i > 0) && !iswspace(*itr) && iswspace(*prevItr))
            {
                numSpaces++;
            }
        }
		
        space = extraSpace/numSpaces;
    }
	
	// Output all characters of the string
    FTUnicodeStringItr<T> prevItr(string), itr(string);
    for(int i = 0; ((len < 0) && *itr) || ((len >= 0) && (i <= len));
        ++i, prevItr = itr++)
    {
        // If this is the end of a space block, distribute the extra space
        // inside it
        if((i > 0) && !iswspace(*itr) && iswspace(*prevItr))
        {
            pen += difont::Point(space, 0);
        }
		
		/**
		 * We want to minimise repeated calls to this function -- is it possible to
		 * set up a cache using Advance?
		 */
		pen = currentFont->Render(itr.getBufferFromHere(), 1, pen, difont::Point(), renderMode);
    }
}


void SimpleLayoutImpl::RenderSpace(const char *string, const int len,
                                     difont::Point position, int renderMode,
                                     const float extraSpace)
{
    RenderSpaceI(string, len, position, renderMode, extraSpace);
}


void SimpleLayoutImpl::RenderSpace(const wchar_t *string, const int len,
                                     difont::Point position, int renderMode,
                                     const float extraSpace)
{
    RenderSpaceI(string, len, position, renderMode, extraSpace);
}

