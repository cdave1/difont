//-----------------------------------------------------------------------------
// Copyright (c) 2009 dhpoware. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include <windows.h>
#include <GL/gl.h>
#include <cassert>

#include "WGL_EXT_swap_control.h"

#define LOAD_ENTRYPOINT(name, var, type) \
    if (!var) \
    { \
        var = reinterpret_cast<type>(wglGetProcAddress(name)); \
        assert(var != 0); \
    }

BOOL wglSwapIntervalEXT(int interval)
{
    typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int interval);
    static PFNWGLSWAPINTERVALEXTPROC pfnSwapIntervalEXT = 0;
    LOAD_ENTRYPOINT("wglSwapIntervalEXT", pfnSwapIntervalEXT, PFNWGLSWAPINTERVALEXTPROC);
    return pfnSwapIntervalEXT(interval);
}

int wglGetSwapIntervalEXT(void)
{
    typedef int (WINAPI * PFNWGLGETSWAPINTERVALEXTPROC)(void);
    static PFNWGLGETSWAPINTERVALEXTPROC pfnGetSwapIntervalEXT = 0;
    LOAD_ENTRYPOINT("wglGetSwapIntervalEXT", pfnGetSwapIntervalEXT, PFNWGLGETSWAPINTERVALEXTPROC);
    return pfnGetSwapIntervalEXT();
}