//-----------------------------------------------------------------------------
// Copyright (c) 2007-2009 dhpoware. All Rights Reserved.
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

#include <cassert>
#include <cstring>
#include <cstdio>
#include <string>
#include <sstream>
#include <set>
#include "gl.h"

namespace
{
	WNDCLASSEX g_wcl;
	HWND g_hWnd;
	HDC g_hDC;
	HGLRC g_hRC;

	LRESULT CALLBACK DummyGLWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CREATE:
			if (!(g_hDC = GetDC(hWnd)))
				return -1;
			break;

		case WM_DESTROY:
			if (g_hDC)
			{
				if (g_hRC)
				{
					wglMakeCurrent(g_hDC, 0);
					wglDeleteContext(g_hRC);
					g_hRC = 0;
				}

				ReleaseDC(hWnd, g_hDC);
				g_hDC = 0;
			}

			PostQuitMessage(0);
			return 0;

		default:
			break;
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	bool CreateDummyGLWindow()
	{
		g_wcl.cbSize = sizeof(g_wcl);
		g_wcl.style = CS_OWNDC;
		g_wcl.lpfnWndProc = DummyGLWndProc;
		g_wcl.hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(0));
		g_wcl.lpszClassName = "DummyGLWindowClass";

		if (!RegisterClassEx(&g_wcl))
			return false;

		g_hWnd = CreateWindow(g_wcl.lpszClassName, "", WS_OVERLAPPEDWINDOW,
					0, 0, 0, 0, 0, 0, g_wcl.hInstance, 0);

		if (!g_hWnd)
			return false;

		PIXELFORMATDESCRIPTOR pfd = {0};

		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 24;
		pfd.cDepthBits = 16;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int pf = ChoosePixelFormat(g_hDC, &pfd);

		if (!SetPixelFormat(g_hDC, pf, &pfd))
			return false;

		if (!(g_hRC = wglCreateContext(g_hDC)))
			return false;

		if (!wglMakeCurrent(g_hDC, g_hRC))
			return false;

		return true;
	}

	void DestroyDummyGLWindow()
	{
		if (g_hWnd)
		{
			PostMessage(g_hWnd, WM_CLOSE, 0, 0);

			BOOL bRet;
			MSG msg;

			while ((bRet = GetMessage(&msg, 0, 0, 0)) != 0)
			{ 
				TranslateMessage(&msg); 
				DispatchMessage(&msg); 
			}
		}        

		UnregisterClass(g_wcl.lpszClassName, g_wcl.hInstance);
	}
}

namespace gl
{
    bool ExtensionSupported(const char *pszExtensionName)
    {
        static std::set<std::string> extensions;       

        if (extensions.empty())
        {
            if (!CreateDummyGLWindow())
            {
                DestroyDummyGLWindow();
                return false;
            }

            // The dummy window will only be at most an OpenGL 2.1 rendering
            // context so we can still use the deprecated GL_EXTENSIONS flag
            // to glGetString().

            if (const char *pszExtensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)))
            {
                std::string str(pszExtensions);
                std::istringstream is(str);
                
                while (is >> str)
                    extensions.insert(str);
            }
            
            // WGL_ARB_extensions_string.

            typedef const char *(WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC);

            PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB =
                reinterpret_cast<PFNWGLGETEXTENSIONSSTRINGARBPROC>(
                wglGetProcAddress("wglGetExtensionsStringARB"));

            if (wglGetExtensionsStringARB)
            {
                std::string str(wglGetExtensionsStringARB(wglGetCurrentDC()));
                std::istringstream is(str);
                
                while (is >> str)
                    extensions.insert(str);
            }

            DestroyDummyGLWindow();
        }

        if (extensions.find(pszExtensionName) == extensions.end())
            return false;

        return true;
    }

    void GetGLVersion(int &major, int &minor)
    {
        static int majorGL = 0;
        static int minorGL = 0;

        if (!majorGL && !minorGL)
        {
            const char *pszVersion = reinterpret_cast<const char *>(glGetString(GL_VERSION));

            if (pszVersion)
                sscanf_s(pszVersion, "%d.%d", &majorGL, &minorGL);
        }

        major = majorGL;
        minor = minorGL;
    }

    void GetGLSLVersion(int &major, int &minor)
    {
        static int majorGLSL = 0;
        static int minorGLSL = 0;

        if (!majorGLSL && !minorGLSL)
        {
            int majorGL = 0;
            int minorGL = 0;

            GetGLVersion(majorGL, minorGL);

            if (majorGL >= 2)
            {
                const char *pszShaderVersion = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));

                if (pszShaderVersion)
                    sscanf_s(pszShaderVersion, "%d.%d", &majorGLSL, &minorGLSL);
            }
            else
            {
                const char *pszExtension = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));

                if (pszExtension)
                {
                    if (strstr(pszExtension, "GL_ARB_shading_language_100"))
                    {
                        majorGLSL = 1;
                        minorGLSL = 0;
                    }
                }
            }
        }

        major = majorGLSL;
        minor = minorGLSL;
    }

    bool SupportsGLVersion(int major, int minor)
    {
        static int majorGL = 0;
        static int minorGL = 0;

        if (!majorGL && !minorGL)
            GetGLVersion(majorGL, minorGL);

        if (majorGL > major)
            return true;

        if (majorGL == major && minorGL >= minor)
            return true;

        return false;
    }

    bool SupportsGLSLVersion(int major, int minor)
    {
        static int majorGLSL = 0;
        static int minorGLSL = 0;

        if (!majorGLSL && !minorGLSL)
            GetGLSLVersion(majorGLSL, minorGLSL);

        if (majorGLSL > major)
            return true;

        if (majorGLSL == major && minorGLSL >= minor)
            return true;

        return false;
    }
}

#define LOAD_ENTRYPOINT(name, var, type) \
    if (!var) \
    { \
        var = reinterpret_cast<type>(wglGetProcAddress(name)); \
        assert(var != 0); \
    }

//
// OpenGL 1.2
//

void glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    typedef void (APIENTRY * PFNGLBLENDCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
    static PFNGLBLENDCOLORPROC pfnBlendColor = 0;
    LOAD_ENTRYPOINT("glBlendColor", pfnBlendColor, PFNGLBLENDCOLORPROC);
    pfnBlendColor(red, green, blue, alpha);
}

void glBlendEquation(GLenum mode)
{
    typedef void (APIENTRY * PFNGLBLENDEQUATIONPROC) (GLenum mode);
    static PFNGLBLENDEQUATIONPROC pfnBlendEquation = 0;
    LOAD_ENTRYPOINT("glBlendEquation", pfnBlendEquation, PFNGLBLENDEQUATIONPROC);
    pfnBlendEquation(mode);
}

void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
    typedef void (APIENTRY * PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
    static PFNGLDRAWRANGEELEMENTSPROC pfnDrawRangeElements = 0;
    LOAD_ENTRYPOINT("glDrawRangeElements", pfnDrawRangeElements, PFNGLDRAWRANGEELEMENTSPROC);
    pfnDrawRangeElements(mode, start, end, count, type, indices);
}

void glColorTable(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table)
{
    typedef void (APIENTRY * PFNGLCOLORTABLEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
    static PFNGLCOLORTABLEPROC pfnColorTable = 0;
    LOAD_ENTRYPOINT("glColorTable", pfnColorTable, PFNGLCOLORTABLEPROC);
    pfnColorTable(target, internalformat, width, format, type, table);
}

void glColorTableParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    typedef void (APIENTRY * PFNGLCOLORTABLEPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
    static PFNGLCOLORTABLEPARAMETERFVPROC pfnColorTableParameterfv = 0;
    LOAD_ENTRYPOINT("glColorTableParameterfv", pfnColorTableParameterfv, PFNGLCOLORTABLEPARAMETERFVPROC);
    pfnColorTableParameterfv(target, pname, params);
}

void glColorTableParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    typedef void (APIENTRY * PFNGLCOLORTABLEPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
    static PFNGLCOLORTABLEPARAMETERIVPROC pfnColorTableParameteriv = 0;
    LOAD_ENTRYPOINT("glColorTableParameteriv", pfnColorTableParameteriv, PFNGLCOLORTABLEPARAMETERIVPROC);
    pfnColorTableParameteriv(target, pname, params);
}

void glCopyColorTable(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    typedef void (APIENTRY * PFNGLCOPYCOLORTABLEPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
    static PFNGLCOPYCOLORTABLEPROC pfnCopyColorTable = 0;
    LOAD_ENTRYPOINT("glCopyColorTable", pfnCopyColorTable, PFNGLCOPYCOLORTABLEPROC);
    pfnCopyColorTable(target, internalformat, x, y, width);
}

void glGetColorTable(GLenum target, GLenum format, GLenum type, GLvoid *table)
{
    typedef void (APIENTRY * PFNGLGETCOLORTABLEPROC) (GLenum target, GLenum format, GLenum type, GLvoid *table);
    static PFNGLGETCOLORTABLEPROC pfnGetColorTable = 0;
    LOAD_ENTRYPOINT("glGetColorTable", pfnGetColorTable, PFNGLGETCOLORTABLEPROC);
    pfnGetColorTable(target, format, type, table);
}

void glGetColorTableParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    typedef void (APIENTRY * PFNGLGETCOLORTABLEPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
    static PFNGLGETCOLORTABLEPARAMETERFVPROC pfnGetColorTableParameterfv = 0;
    LOAD_ENTRYPOINT("glGetColorTableParameterfv", pfnGetColorTableParameterfv, PFNGLGETCOLORTABLEPARAMETERFVPROC);
    pfnGetColorTableParameterfv(target, pname, params);
}

void glGetColorTableParameteriv(GLenum target, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETCOLORTABLEPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
    static PFNGLGETCOLORTABLEPARAMETERIVPROC pfnGetColorTableParameteriv = 0;
    LOAD_ENTRYPOINT("glGetColorTableParameteriv", pfnGetColorTableParameteriv, PFNGLGETCOLORTABLEPARAMETERIVPROC);
    pfnGetColorTableParameteriv(target, pname, params);
}

void glColorSubTable(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLCOLORSUBTABLEPROC) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
    static PFNGLCOLORSUBTABLEPROC pfnColorSubTable = 0;
    LOAD_ENTRYPOINT("glColorSubTable", pfnColorSubTable, PFNGLCOLORSUBTABLEPROC);
    pfnColorSubTable(target, start, count, format, type, data);
}

void glCopyColorSubTable(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)
{
    typedef void (APIENTRY * PFNGLCOPYCOLORSUBTABLEPROC) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
    static PFNGLCOPYCOLORSUBTABLEPROC pfnCopyColorSubTable = 0;
    LOAD_ENTRYPOINT("glCopyColorSubTable", pfnCopyColorSubTable, PFNGLCOPYCOLORSUBTABLEPROC);
    pfnCopyColorSubTable(target, start, x, y, width);
}

void glConvolutionFilter1D(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image)
{
    typedef void (APIENTRY * PFNGLCONVOLUTIONFILTER1DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
    static PFNGLCONVOLUTIONFILTER1DPROC pfnConvolutionFilter1D = 0;
    LOAD_ENTRYPOINT("glConvolutionFilter1D", pfnConvolutionFilter1D, PFNGLCONVOLUTIONFILTER1DPROC);
    pfnConvolutionFilter1D(target, internalformat, width, format, type, image);
}

void glConvolutionFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image)
{
    typedef void (APIENTRY * PFNGLCONVOLUTIONFILTER2DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
    static PFNGLCONVOLUTIONFILTER2DPROC pfnConvolutionFilter2D = 0;
    LOAD_ENTRYPOINT("glConvolutionFilter2D", pfnConvolutionFilter2D, PFNGLCONVOLUTIONFILTER2DPROC);
    pfnConvolutionFilter2D(target, internalformat, width, height, format, type, image);
}

void glConvolutionParameterf(GLenum target, GLenum pname, GLfloat params)
{
    typedef void (APIENTRY * PFNGLCONVOLUTIONPARAMETERFPROC) (GLenum target, GLenum pname, GLfloat params);
    static PFNGLCONVOLUTIONPARAMETERFPROC pfnConvolutionParameterf = 0;
    LOAD_ENTRYPOINT("glConvolutionParameterf", pfnConvolutionParameterf, PFNGLCONVOLUTIONPARAMETERFPROC);
    pfnConvolutionParameterf(target, pname, params);
}

void glConvolutionParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    typedef void (APIENTRY * PFNGLCONVOLUTIONPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
    static PFNGLCONVOLUTIONPARAMETERFVPROC pfnConvolutionParameterfv = 0;
    LOAD_ENTRYPOINT("glConvolutionParameterfv", pfnConvolutionParameterfv, PFNGLCONVOLUTIONPARAMETERFVPROC);
    pfnConvolutionParameterfv(target, pname, params);
}

void glConvolutionParameteri(GLenum target, GLenum pname, GLint params)
{
    typedef void (APIENTRY * PFNGLCONVOLUTIONPARAMETERIPROC) (GLenum target, GLenum pname, GLint params);
    static PFNGLCONVOLUTIONPARAMETERIPROC pfnConvolutionParameteri = 0;
    LOAD_ENTRYPOINT("glConvolutionParameteri", pfnConvolutionParameteri, PFNGLCONVOLUTIONPARAMETERIPROC);
    pfnConvolutionParameteri(target, pname, params);
}

void glConvolutionParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    typedef void (APIENTRY * PFNGLCONVOLUTIONPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
    static PFNGLCONVOLUTIONPARAMETERIVPROC pfnConvolutionParameteriv = 0;
    LOAD_ENTRYPOINT("glConvolutionParameteriv", pfnConvolutionParameteriv, PFNGLCONVOLUTIONPARAMETERIVPROC);
    pfnConvolutionParameteriv(target, pname, params);
}

void glCopyConvolutionFilter1D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    typedef void (APIENTRY * PFNGLCOPYCONVOLUTIONFILTER1DPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
    static PFNGLCOPYCONVOLUTIONFILTER1DPROC pfnCopyConvolutionFilter1D = 0;
    LOAD_ENTRYPOINT("glCopyConvolutionFilter1D", pfnCopyConvolutionFilter1D, PFNGLCOPYCONVOLUTIONFILTER1DPROC);
    pfnCopyConvolutionFilter1D(target, internalformat, x, y, width);
}

void glCopyConvolutionFilter2D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)
{
    typedef void (APIENTRY * PFNGLCOPYCONVOLUTIONFILTER2DPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
    static PFNGLCOPYCONVOLUTIONFILTER2DPROC pfnCopyConvolutionFilter2D = 0;
    LOAD_ENTRYPOINT("glCopyConvolutionFilter2D", pfnCopyConvolutionFilter2D, PFNGLCOPYCONVOLUTIONFILTER2DPROC);
    pfnCopyConvolutionFilter2D(target, internalformat, x, y, width, height);
}

void glGetConvolutionFilter(GLenum target, GLenum format, GLenum type, GLvoid *image)
{
    typedef void (APIENTRY * PFNGLGETCONVOLUTIONFILTERPROC) (GLenum target, GLenum format, GLenum type, GLvoid *image);
    static PFNGLGETCONVOLUTIONFILTERPROC pfnGetConvolutionFilter = 0;
    LOAD_ENTRYPOINT("glGetConvolutionFilter", pfnGetConvolutionFilter, PFNGLGETCONVOLUTIONFILTERPROC);
    pfnGetConvolutionFilter(target, format, type, image);
}

void glGetConvolutionParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    typedef void (APIENTRY * PFNGLGETCONVOLUTIONPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
    static PFNGLGETCONVOLUTIONPARAMETERFVPROC pfnGetConvolutionParameterfv = 0;
    LOAD_ENTRYPOINT("glGetConvolutionParameterfv", pfnGetConvolutionParameterfv, PFNGLGETCONVOLUTIONPARAMETERFVPROC);
    pfnGetConvolutionParameterfv(target, pname, params);
}

void glGetConvolutionParameteriv(GLenum target, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETCONVOLUTIONPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
    static PFNGLGETCONVOLUTIONPARAMETERIVPROC pfnGetConvolutionParameteriv = 0;
    LOAD_ENTRYPOINT("glGetConvolutionParameteriv", pfnGetConvolutionParameteriv, PFNGLGETCONVOLUTIONPARAMETERIVPROC);
    pfnGetConvolutionParameteriv(target, pname, params);
}

void glGetSeparableFilter(GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span)
{
    typedef void (APIENTRY * PFNGLGETSEPARABLEFILTERPROC) (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
    static PFNGLGETSEPARABLEFILTERPROC pfnGetSeparableFilter = 0;
    LOAD_ENTRYPOINT("glGetSeparableFilter", pfnGetSeparableFilter, PFNGLGETSEPARABLEFILTERPROC);
    pfnGetSeparableFilter(target, format, type, row, column, span);
}

void glSeparableFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column)
{
    typedef void (APIENTRY * PFNGLSEPARABLEFILTER2DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
    static PFNGLSEPARABLEFILTER2DPROC pfnSeparableFilter2D = 0;
    LOAD_ENTRYPOINT("glSeparableFilter2D", pfnSeparableFilter2D, PFNGLSEPARABLEFILTER2DPROC);
    pfnSeparableFilter2D(target, internalformat, width, height, format, type, row, column);
}

void glGetHistogram(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
    typedef void (APIENTRY * PFNGLGETHISTOGRAMPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
    static PFNGLGETHISTOGRAMPROC pfnGetHistogram = 0;
    LOAD_ENTRYPOINT("glGetHistogram", pfnGetHistogram, PFNGLGETHISTOGRAMPROC);
    pfnGetHistogram(target, reset, format, type, values);
}

void glGetHistogramParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    typedef void (APIENTRY * PFNGLGETHISTOGRAMPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
    static PFNGLGETHISTOGRAMPARAMETERFVPROC pfnGetHistogramParameterfv = 0;
    LOAD_ENTRYPOINT("glGetHistogramParameterfv", pfnGetHistogramParameterfv, PFNGLGETHISTOGRAMPARAMETERFVPROC);
    pfnGetHistogramParameterfv(target, pname, params);
}

void glGetHistogramParameteriv(GLenum target, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETHISTOGRAMPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
    static PFNGLGETHISTOGRAMPARAMETERIVPROC pfnGetHistogramParameteriv = 0;
    LOAD_ENTRYPOINT("glGetHistogramParameteriv", pfnGetHistogramParameteriv, PFNGLGETHISTOGRAMPARAMETERIVPROC);
    pfnGetHistogramParameteriv(target, pname, params);
}

void glGetMinmax(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
    typedef void (APIENTRY * PFNGLGETMINMAXPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
    static PFNGLGETMINMAXPROC pfnGetMinmax = 0;
    LOAD_ENTRYPOINT("glGetMinmax", pfnGetMinmax, PFNGLGETMINMAXPROC);
    pfnGetMinmax(target, reset, format, type, values);
}

void glGetMinmaxParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    typedef void (APIENTRY * PFNGLGETMINMAXPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
    static PFNGLGETMINMAXPARAMETERFVPROC pfnGetMinmaxParameterfv = 0;
    LOAD_ENTRYPOINT("glGetMinmaxParameterfv", pfnGetMinmaxParameterfv, PFNGLGETMINMAXPARAMETERFVPROC);
    pfnGetMinmaxParameterfv(target, pname, params);
}

void glGetMinmaxParameteriv(GLenum target, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETMINMAXPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
    static PFNGLGETMINMAXPARAMETERIVPROC pfnGetMinmaxParameteriv = 0;
    LOAD_ENTRYPOINT("glGetMinmaxParameteriv", pfnGetMinmaxParameteriv, PFNGLGETMINMAXPARAMETERIVPROC);
    pfnGetMinmaxParameteriv(target, pname, params);
}

void glHistogram(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)
{
    typedef void (APIENTRY * PFNGLHISTOGRAMPROC) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
    static PFNGLHISTOGRAMPROC pfnHistogram = 0;
    LOAD_ENTRYPOINT("glHistogram", pfnHistogram, PFNGLHISTOGRAMPROC);
    pfnHistogram(target, width, internalformat, sink);
}

void glMinmax(GLenum target, GLenum internalformat, GLboolean sink)
{
    typedef void (APIENTRY * PFNGLMINMAXPROC) (GLenum target, GLenum internalformat, GLboolean sink);
    static PFNGLMINMAXPROC pfnMinmax = 0;
    LOAD_ENTRYPOINT("glMinmax", pfnMinmax, PFNGLMINMAXPROC);
    pfnMinmax(target, internalformat, sink);
}

void glResetHistogram(GLenum target)
{
    typedef void (APIENTRY * PFNGLRESETHISTOGRAMPROC) (GLenum target);
    static PFNGLRESETHISTOGRAMPROC pfnResetHistogram = 0;
    LOAD_ENTRYPOINT("glResetHistogram", pfnResetHistogram, PFNGLRESETHISTOGRAMPROC);
    pfnResetHistogram(target);
}

void glResetMinmax(GLenum target)
{
    typedef void (APIENTRY * PFNGLRESETMINMAXPROC) (GLenum target);
    static PFNGLRESETMINMAXPROC pfnResetMinmax = 0;
    LOAD_ENTRYPOINT("glResetMinmax", pfnResetMinmax, PFNGLRESETMINMAXPROC);
    pfnResetMinmax(target);
}

void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    typedef void (APIENTRY * PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    static PFNGLTEXIMAGE3DPROC pfnTexImage3D = 0;
    LOAD_ENTRYPOINT("glTexImage3D", pfnTexImage3D, PFNGLTEXIMAGE3DPROC);
    pfnTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
}

void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
    typedef void (APIENTRY * PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
    static PFNGLTEXSUBIMAGE3DPROC pfnTexSubImage3D = 0;
    LOAD_ENTRYPOINT("glTexSubImage3D", pfnTexSubImage3D, PFNGLTEXSUBIMAGE3DPROC);
    pfnTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    typedef void (APIENTRY * PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    static PFNGLCOPYTEXSUBIMAGE3DPROC pfnCopyTexSubImage3D = 0;
    LOAD_ENTRYPOINT("glCopyTexSubImage3D", pfnCopyTexSubImage3D, PFNGLCOPYTEXSUBIMAGE3DPROC);
    pfnCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

//
// OpenGL 1.3
//

void glActiveTexture(GLenum texture)
{
    typedef void (APIENTRY * PFNGLACTIVETEXTUREPROC) (GLenum texture);
    static PFNGLACTIVETEXTUREPROC pfnActiveTexture = 0;
    LOAD_ENTRYPOINT("glActiveTexture", pfnActiveTexture, PFNGLACTIVETEXTUREPROC);
    pfnActiveTexture(texture);
}

void glClientActiveTexture(GLenum texture)
{
    typedef void (APIENTRY * PFNGLCLIENTACTIVETEXTUREPROC) (GLenum texture);
    static PFNGLCLIENTACTIVETEXTUREPROC pfnClientActiveTexture = 0;
    LOAD_ENTRYPOINT("glClientActiveTexture", pfnClientActiveTexture, PFNGLCLIENTACTIVETEXTUREPROC);
    pfnClientActiveTexture(texture);
}

void glMultiTexCoord1d(GLenum target, GLdouble s)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD1DPROC) (GLenum target, GLdouble s);
    static PFNGLMULTITEXCOORD1DPROC pfnMultiTexCoord1d = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord1d", pfnMultiTexCoord1d, PFNGLMULTITEXCOORD1DPROC);
    pfnMultiTexCoord1d(target, s);
}

void glMultiTexCoord1dv(GLenum target, const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD1DVPROC) (GLenum target, const GLdouble *v);
    static PFNGLMULTITEXCOORD1DVPROC pfnMultiTexCoord1dv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord1dv", pfnMultiTexCoord1dv, PFNGLMULTITEXCOORD1DVPROC);
    pfnMultiTexCoord1dv(target, v);
}

void glMultiTexCoord1f(GLenum target, GLfloat s)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD1FPROC) (GLenum target, GLfloat s);
    static PFNGLMULTITEXCOORD1FPROC pfnMultiTexCoord1f = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord1f", pfnMultiTexCoord1f, PFNGLMULTITEXCOORD1FPROC);
    pfnMultiTexCoord1f(target, s);
}

void glMultiTexCoord1fv(GLenum target, const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD1FVPROC) (GLenum target, const GLfloat *v);
    static PFNGLMULTITEXCOORD1FVPROC pfnMultiTexCoord1fv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord1fv", pfnMultiTexCoord1fv, PFNGLMULTITEXCOORD1FVPROC);
    pfnMultiTexCoord1fv(target, v);
}

void glMultiTexCoord1i(GLenum target, GLint s)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD1IPROC) (GLenum target, GLint s);
    static PFNGLMULTITEXCOORD1IPROC pfnMultiTexCoord1i = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord1i", pfnMultiTexCoord1i, PFNGLMULTITEXCOORD1IPROC);
    pfnMultiTexCoord1i(target, s);
}

void glMultiTexCoord1iv(GLenum target, const GLint *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD1IVPROC) (GLenum target, const GLint *v);
    static PFNGLMULTITEXCOORD1IVPROC pfnMultiTexCoord1iv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord1iv", pfnMultiTexCoord1iv, PFNGLMULTITEXCOORD1IVPROC);
    pfnMultiTexCoord1iv(target, v);
}

void glMultiTexCoord1s(GLenum target, GLshort s)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD1SPROC) (GLenum target, GLshort s);
    static PFNGLMULTITEXCOORD1SPROC pfnMultiTexCoord1s = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord1s", pfnMultiTexCoord1s, PFNGLMULTITEXCOORD1SPROC);
    pfnMultiTexCoord1s(target, s);
}

void glMultiTexCoord1sv(GLenum target, const GLshort *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD1SVPROC) (GLenum target, const GLshort *v);
    static PFNGLMULTITEXCOORD1SVPROC pfnMultiTexCoord1sv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord1sv", pfnMultiTexCoord1sv, PFNGLMULTITEXCOORD1SVPROC);
    pfnMultiTexCoord1sv(target, v);
}

void glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD2DPROC) (GLenum target, GLdouble s, GLdouble t);
    static PFNGLMULTITEXCOORD2DPROC pfnMultiTexCoord2d = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord2d", pfnMultiTexCoord2d, PFNGLMULTITEXCOORD2DPROC);
    pfnMultiTexCoord2d(target, s, t);
}

void glMultiTexCoord2dv(GLenum target, const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD2DVPROC) (GLenum target, const GLdouble *v);
    static PFNGLMULTITEXCOORD2DVPROC pfnMultiTexCoord2dv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord2dv", pfnMultiTexCoord2dv, PFNGLMULTITEXCOORD2DVPROC);
    pfnMultiTexCoord2dv(target, v);
}

void glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD2FPROC) (GLenum target, GLfloat s, GLfloat t);
    static PFNGLMULTITEXCOORD2FPROC pfnMultiTexCoord2f = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord2f", pfnMultiTexCoord2f, PFNGLMULTITEXCOORD2FPROC);
    pfnMultiTexCoord2f(target, s, t);
}

void glMultiTexCoord2fv(GLenum target, const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD2FVPROC) (GLenum target, const GLfloat *v);
    static PFNGLMULTITEXCOORD2FVPROC pfnMultiTexCoord2fv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord2fv", pfnMultiTexCoord2fv, PFNGLMULTITEXCOORD2FVPROC);
    pfnMultiTexCoord2fv(target, v);
}

void glMultiTexCoord2i(GLenum target, GLint s, GLint t)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD2IPROC) (GLenum target, GLint s, GLint t);
    static PFNGLMULTITEXCOORD2IPROC pfnMultiTexCoord2i = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord2i", pfnMultiTexCoord2i, PFNGLMULTITEXCOORD2IPROC);
    pfnMultiTexCoord2i(target, s, t);
}

void glMultiTexCoord2iv(GLenum target, const GLint *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD2IVPROC) (GLenum target, const GLint *v);
    static PFNGLMULTITEXCOORD2IVPROC pfnMultiTexCoord2iv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord2iv", pfnMultiTexCoord2iv, PFNGLMULTITEXCOORD2IVPROC);
    pfnMultiTexCoord2iv(target, v);
}

void glMultiTexCoord2s(GLenum target, GLshort s, GLshort t)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD2SPROC) (GLenum target, GLshort s, GLshort t);
    static PFNGLMULTITEXCOORD2SPROC pfnMultiTexCoord2s = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord2s", pfnMultiTexCoord2s, PFNGLMULTITEXCOORD2SPROC);
    pfnMultiTexCoord2s(target, s, t);
}

void glMultiTexCoord2sv(GLenum target, const GLshort *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD2SVPROC) (GLenum target, const GLshort *v);
    static PFNGLMULTITEXCOORD2SVPROC pfnMultiTexCoord2sv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord2sv", pfnMultiTexCoord2sv, PFNGLMULTITEXCOORD2SVPROC);
    pfnMultiTexCoord2sv(target, v);
}

void glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD3DPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
    static PFNGLMULTITEXCOORD3DPROC pfnMultiTexCoord3d = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord3d", pfnMultiTexCoord3d, PFNGLMULTITEXCOORD3DPROC);
    pfnMultiTexCoord3d(target, s, t, r);
}

void glMultiTexCoord3dv(GLenum target, const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD3DVPROC) (GLenum target, const GLdouble *v);
    static PFNGLMULTITEXCOORD3DVPROC pfnMultiTexCoord3dv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord3dv", pfnMultiTexCoord3dv, PFNGLMULTITEXCOORD3DVPROC);
    pfnMultiTexCoord3dv(target, v);
}

void glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD3FPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
    static PFNGLMULTITEXCOORD3FPROC pfnMultiTexCoord3f = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord3f", pfnMultiTexCoord3f, PFNGLMULTITEXCOORD3FPROC);
    pfnMultiTexCoord3f(target, s, t, r);
}

void glMultiTexCoord3fv(GLenum target, const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD3FVPROC) (GLenum target, const GLfloat *v);
    static PFNGLMULTITEXCOORD3FVPROC pfnMultiTexCoord3fv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord3fv", pfnMultiTexCoord3fv, PFNGLMULTITEXCOORD3FVPROC);
    pfnMultiTexCoord3fv(target, v);
}

void glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD3IPROC) (GLenum target, GLint s, GLint t, GLint r);
    static PFNGLMULTITEXCOORD3IPROC pfnMultiTexCoord3i = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord3i", pfnMultiTexCoord3i, PFNGLMULTITEXCOORD3IPROC);
    pfnMultiTexCoord3i(target, s, t, r);
}

void glMultiTexCoord3iv(GLenum target, const GLint *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD3IVPROC) (GLenum target, const GLint *v);
    static PFNGLMULTITEXCOORD3IVPROC pfnMultiTexCoord3iv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord3iv", pfnMultiTexCoord3iv, PFNGLMULTITEXCOORD3IVPROC);
    pfnMultiTexCoord3iv(target, v);
}

void glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD3SPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
    static PFNGLMULTITEXCOORD3SPROC pfnMultiTexCoord3s = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord3s", pfnMultiTexCoord3s, PFNGLMULTITEXCOORD3SPROC);
    pfnMultiTexCoord3s(target, s, t, r);
}

void glMultiTexCoord3sv(GLenum target, const GLshort *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD3SVPROC) (GLenum target, const GLshort *v);
    static PFNGLMULTITEXCOORD3SVPROC pfnMultiTexCoord3sv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord3sv", pfnMultiTexCoord3sv, PFNGLMULTITEXCOORD3SVPROC);
    pfnMultiTexCoord3sv(target, v);
}

void glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD4DPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
    static PFNGLMULTITEXCOORD4DPROC pfnMultiTexCoord4d = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord4d", pfnMultiTexCoord4d, PFNGLMULTITEXCOORD4DPROC);
    pfnMultiTexCoord4d(target, s, t, r, q);
}

void glMultiTexCoord4dv(GLenum target, const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD4DVPROC) (GLenum target, const GLdouble *v);
    static PFNGLMULTITEXCOORD4DVPROC pfnMultiTexCoord4dv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord4dv", pfnMultiTexCoord4dv, PFNGLMULTITEXCOORD4DVPROC);
    pfnMultiTexCoord4dv(target, v);
}

void glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD4FPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    static PFNGLMULTITEXCOORD4FPROC pfnMultiTexCoord4f = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord4f", pfnMultiTexCoord4f, PFNGLMULTITEXCOORD4FPROC);
    pfnMultiTexCoord4f(target, s, t, r, q);
}

void glMultiTexCoord4fv(GLenum target, const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD4FVPROC) (GLenum target, const GLfloat *v);
    static PFNGLMULTITEXCOORD4FVPROC pfnMultiTexCoord4fv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord4fv", pfnMultiTexCoord4fv, PFNGLMULTITEXCOORD4FVPROC);
    pfnMultiTexCoord4fv(target, v);
}

void glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD4IPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
    static PFNGLMULTITEXCOORD4IPROC pfnMultiTexCoord4i = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord4i", pfnMultiTexCoord4i, PFNGLMULTITEXCOORD4IPROC);
    pfnMultiTexCoord4i(target, s, t, r, q);
}

void glMultiTexCoord4iv(GLenum target, const GLint *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD4IVPROC) (GLenum target, const GLint *v);
    static PFNGLMULTITEXCOORD4IVPROC pfnMultiTexCoord4iv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord4iv", pfnMultiTexCoord4iv, PFNGLMULTITEXCOORD4IVPROC);
    pfnMultiTexCoord4iv(target, v);
}

void glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD4SPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
    static PFNGLMULTITEXCOORD4SPROC pfnMultiTexCoord4s = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord4s", pfnMultiTexCoord4s, PFNGLMULTITEXCOORD4SPROC);
    pfnMultiTexCoord4s(target, s, t, r, q);
}

void glMultiTexCoord4sv(GLenum target, const GLshort *v)
{
    typedef void (APIENTRY * PFNGLMULTITEXCOORD4SVPROC) (GLenum target, const GLshort *v);
    static PFNGLMULTITEXCOORD4SVPROC pfnMultiTexCoord4sv = 0;
    LOAD_ENTRYPOINT("glMultiTexCoord4sv", pfnMultiTexCoord4sv, PFNGLMULTITEXCOORD4SVPROC);
    pfnMultiTexCoord4sv(target, v);
}

void glLoadTransposeMatrixf(const GLfloat *m)
{
    typedef void (APIENTRY * PFNGLLOADTRANSPOSEMATRIXFPROC) (const GLfloat *m);
    static PFNGLLOADTRANSPOSEMATRIXFPROC pfnLoadTransposeMatrixf = 0;
    LOAD_ENTRYPOINT("glLoadTransposeMatrixf", pfnLoadTransposeMatrixf, PFNGLLOADTRANSPOSEMATRIXFPROC);
    pfnLoadTransposeMatrixf(m);
}

void glLoadTransposeMatrixd(const GLdouble *m)
{
    typedef void (APIENTRY * PFNGLLOADTRANSPOSEMATRIXDPROC) (const GLdouble *m);
    static PFNGLLOADTRANSPOSEMATRIXDPROC pfnLoadTransposeMatrixd = 0;
    LOAD_ENTRYPOINT("glLoadTransposeMatrixd", pfnLoadTransposeMatrixd, PFNGLLOADTRANSPOSEMATRIXDPROC);
    pfnLoadTransposeMatrixd(m);
}

void glMultTransposeMatrixf(const GLfloat *m)
{
    typedef void (APIENTRY * PFNGLMULTTRANSPOSEMATRIXFPROC) (const GLfloat *m);
    static PFNGLMULTTRANSPOSEMATRIXFPROC pfnMultTransposeMatrixf = 0;
    LOAD_ENTRYPOINT("glMultTransposeMatrixf", pfnMultTransposeMatrixf, PFNGLMULTTRANSPOSEMATRIXFPROC);
    pfnMultTransposeMatrixf(m);
}

void glMultTransposeMatrixd(const GLdouble *m)
{
    typedef void (APIENTRY * PFNGLMULTTRANSPOSEMATRIXDPROC) (const GLdouble *m);
    static PFNGLMULTTRANSPOSEMATRIXDPROC pfnMultTransposeMatrixd = 0;
    LOAD_ENTRYPOINT("glMultTransposeMatrixd", pfnMultTransposeMatrixd, PFNGLMULTTRANSPOSEMATRIXDPROC);
    pfnMultTransposeMatrixd(m);
}

void glSampleCoverage(GLclampf value, GLboolean invert)
{
    typedef void (APIENTRY * PFNGLSAMPLECOVERAGEPROC) (GLclampf value, GLboolean invert);
    static PFNGLSAMPLECOVERAGEPROC pfnSampleCoverage = 0;
    LOAD_ENTRYPOINT("glSampleCoverage", pfnSampleCoverage, PFNGLSAMPLECOVERAGEPROC);
    pfnSampleCoverage(value, invert);
}

void glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE3DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
    static PFNGLCOMPRESSEDTEXIMAGE3DPROC pfnCompressedTexImage3D = 0;
    LOAD_ENTRYPOINT("glCompressedTexImage3D", pfnCompressedTexImage3D, PFNGLCOMPRESSEDTEXIMAGE3DPROC);
    pfnCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
}

void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
    static PFNGLCOMPRESSEDTEXIMAGE2DPROC pfnCompressedTexImage2D = 0;
    LOAD_ENTRYPOINT("glCompressedTexImage2D", pfnCompressedTexImage2D, PFNGLCOMPRESSEDTEXIMAGE2DPROC);
    pfnCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE1DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
    static PFNGLCOMPRESSEDTEXIMAGE1DPROC pfnCompressedTexImage1D = 0;
    LOAD_ENTRYPOINT("glCompressedTexImage1D", pfnCompressedTexImage1D, PFNGLCOMPRESSEDTEXIMAGE1DPROC);
    pfnCompressedTexImage1D(target, level, internalformat, width, border, imageSize, data);
}

void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
    static PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC pfnCompressedTexSubImage3D = 0;
    LOAD_ENTRYPOINT("glCompressedTexSubImage3D", pfnCompressedTexSubImage3D, PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC);
    pfnCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
    static PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC pfnCompressedTexSubImage2D = 0;
    LOAD_ENTRYPOINT("glCompressedTexSubImage2D", pfnCompressedTexSubImage2D, PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC);
    pfnCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
    static PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC pfnCompressedTexSubImage1D = 0;
    LOAD_ENTRYPOINT("glCompressedTexSubImage1D", pfnCompressedTexSubImage1D, PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC);
    pfnCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data);
}

void glGetCompressedTexImage(GLenum target, GLint level, GLvoid *img)
{
    typedef void (APIENTRY * PFNGLGETCOMPRESSEDTEXIMAGEPROC) (GLenum target, GLint level, GLvoid *img);
    static PFNGLGETCOMPRESSEDTEXIMAGEPROC pfnGetCompressedTexImage = 0;
    LOAD_ENTRYPOINT("glGetCompressedTexImage", pfnGetCompressedTexImage, PFNGLGETCOMPRESSEDTEXIMAGEPROC);
    pfnGetCompressedTexImage(target, level, img);
}

//
// OpenGl 1.4
//

void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    typedef void (APIENTRY * PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
    static PFNGLBLENDFUNCSEPARATEPROC pfnBlendFuncSeparate = 0;
    LOAD_ENTRYPOINT("glBlendFuncSeparate", pfnBlendFuncSeparate, PFNGLBLENDFUNCSEPARATEPROC);
    pfnBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

void glFogCoordf(GLfloat coord)
{
    typedef void (APIENTRY * PFNGLFOGCOORDFPROC) (GLfloat coord);
    static PFNGLFOGCOORDFPROC pfnFogCoordf = 0;
    LOAD_ENTRYPOINT("glFogCoordf", pfnFogCoordf, PFNGLFOGCOORDFPROC);
    pfnFogCoordf(coord);
}

void glFogCoordfv(const GLfloat *coord)
{
    typedef void (APIENTRY * PFNGLFOGCOORDFVPROC) (const GLfloat *coord);
    static PFNGLFOGCOORDFVPROC pfnFogCoordfv = 0;
    LOAD_ENTRYPOINT("glFogCoordfv", pfnFogCoordfv, PFNGLFOGCOORDFVPROC);
    pfnFogCoordfv(coord);
}

void glFogCoordd(GLdouble coord)
{
    typedef void (APIENTRY * PFNGLFOGCOORDDPROC) (GLdouble coord);
    static PFNGLFOGCOORDDPROC pfnFogCoordd = 0;
    LOAD_ENTRYPOINT("glFogCoordd", pfnFogCoordd, PFNGLFOGCOORDDPROC);
    pfnFogCoordd(coord);
}

void glFogCoorddv(const GLdouble *coord)
{
    typedef void (APIENTRY * PFNGLFOGCOORDDVPROC) (const GLdouble *coord);
    static PFNGLFOGCOORDDVPROC pfnFogCoorddv = 0;
    LOAD_ENTRYPOINT("glFogCoorddv", pfnFogCoorddv, PFNGLFOGCOORDDVPROC);
    pfnFogCoorddv(coord);
}

void glFogCoordPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    typedef void (APIENTRY * PFNGLFOGCOORDPOINTERPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
    static PFNGLFOGCOORDPOINTERPROC pfnFogCoordPointer = 0;
    LOAD_ENTRYPOINT("glFogCoordPointer", pfnFogCoordPointer, PFNGLFOGCOORDPOINTERPROC);
    pfnFogCoordPointer(type, stride, pointer);
}

void glMultiDrawArrays(GLenum mode, GLint *first, GLsizei *count, GLsizei primcount)
{
    typedef void (APIENTRY * PFNGLMULTIDRAWARRAYSPROC) (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount);
    static PFNGLMULTIDRAWARRAYSPROC pfnMultiDrawArrays = 0;
    LOAD_ENTRYPOINT("glMultiDrawArrays", pfnMultiDrawArrays, PFNGLMULTIDRAWARRAYSPROC);
    pfnMultiDrawArrays(mode, first, count, primcount);
}

void glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount)
{
    typedef void (APIENTRY * PFNGLMULTIDRAWELEMENTSPROC) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
    static PFNGLMULTIDRAWELEMENTSPROC pfnMultiDrawElements = 0;
    LOAD_ENTRYPOINT("glMultiDrawElements", pfnMultiDrawElements, PFNGLMULTIDRAWELEMENTSPROC);
    pfnMultiDrawElements(mode, count, type, indices, primcount);
}

void glPointParameterf(GLenum pname, GLfloat param)
{
    typedef void (APIENTRY * PFNGLPOINTPARAMETERFPROC) (GLenum pname, GLfloat param);
    static PFNGLPOINTPARAMETERFPROC pfnPointParameterf = 0;
    LOAD_ENTRYPOINT("glPointParameterf", pfnPointParameterf, PFNGLPOINTPARAMETERFPROC);
    pfnPointParameterf(pname, param);
}

void glPointParameterfv(GLenum pname, const GLfloat *params)
{
    typedef void (APIENTRY * PFNGLPOINTPARAMETERFVPROC) (GLenum pname, const GLfloat *params);
    static PFNGLPOINTPARAMETERFVPROC pfnPointParameterfv = 0;
    LOAD_ENTRYPOINT("glPointParameterfv", pfnPointParameterfv, PFNGLPOINTPARAMETERFVPROC);
    pfnPointParameterfv(pname, params);
}

void glPointParameteri(GLenum pname, GLint param)
{
    typedef void (APIENTRY * PFNGLPOINTPARAMETERIPROC) (GLenum pname, GLint param);
    static PFNGLPOINTPARAMETERIPROC pfnPointParameteri = 0;
    LOAD_ENTRYPOINT("glPointParameteri", pfnPointParameteri, PFNGLPOINTPARAMETERIPROC);
    pfnPointParameteri(pname, param);
}

void glPointParameteriv(GLenum pname, const GLint *params)
{
    typedef void (APIENTRY * PFNGLPOINTPARAMETERIVPROC) (GLenum pname, const GLint *params);
    static PFNGLPOINTPARAMETERIVPROC pfnPointParameteriv = 0;
    LOAD_ENTRYPOINT("glPointParameteriv", pfnPointParameteriv, PFNGLPOINTPARAMETERIVPROC);
    pfnPointParameteriv(pname, params);
}

void glSecondaryColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3BPROC) (GLbyte red, GLbyte green, GLbyte blue);
    static PFNGLSECONDARYCOLOR3BPROC pfnSecondaryColor3b = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3b", pfnSecondaryColor3b, PFNGLSECONDARYCOLOR3BPROC);
    pfnSecondaryColor3b(red, green, blue);
}

void glSecondaryColor3bv(const GLbyte *v)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3BVPROC) (const GLbyte *v);
    static PFNGLSECONDARYCOLOR3BVPROC pfnSecondaryColor3bv = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3bv", pfnSecondaryColor3bv, PFNGLSECONDARYCOLOR3BVPROC);
    pfnSecondaryColor3bv(v);
}

void glSecondaryColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3DPROC) (GLdouble red, GLdouble green, GLdouble blue);
    static PFNGLSECONDARYCOLOR3DPROC pfnSecondaryColor3d = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3d", pfnSecondaryColor3d, PFNGLSECONDARYCOLOR3DPROC);
    pfnSecondaryColor3d(red, green, blue);
}

void glSecondaryColor3dv(const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3DVPROC) (const GLdouble *v);
    static PFNGLSECONDARYCOLOR3DVPROC pfnSecondaryColor3dv = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3dv", pfnSecondaryColor3dv, PFNGLSECONDARYCOLOR3DVPROC);
    pfnSecondaryColor3dv(v);
}

void glSecondaryColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3FPROC) (GLfloat red, GLfloat green, GLfloat blue);
    static PFNGLSECONDARYCOLOR3FPROC pfnSecondaryColor3f = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3f", pfnSecondaryColor3f, PFNGLSECONDARYCOLOR3FPROC);
    pfnSecondaryColor3f(red, green, blue);
}

void glSecondaryColor3fv(const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3FVPROC) (const GLfloat *v);
    static PFNGLSECONDARYCOLOR3FVPROC pfnSecondaryColor3fv = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3fv", pfnSecondaryColor3fv, PFNGLSECONDARYCOLOR3FVPROC);
    pfnSecondaryColor3fv(v);
}

void glSecondaryColor3i(GLint red, GLint green, GLint blue)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3IPROC) (GLint red, GLint green, GLint blue);
    static PFNGLSECONDARYCOLOR3IPROC pfnSecondaryColor3i = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3i", pfnSecondaryColor3i, PFNGLSECONDARYCOLOR3IPROC);
    pfnSecondaryColor3i(red, green, blue);
}

void glSecondaryColor3iv(const GLint *v)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3IVPROC) (const GLint *v);
    static PFNGLSECONDARYCOLOR3IVPROC pfnSecondaryColor3iv = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3iv", pfnSecondaryColor3iv, PFNGLSECONDARYCOLOR3IVPROC);
    pfnSecondaryColor3iv(v);
}

void glSecondaryColor3s(GLshort red, GLshort green, GLshort blue)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3SPROC) (GLshort red, GLshort green, GLshort blue);
    static PFNGLSECONDARYCOLOR3SPROC pfnSecondaryColor3s = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3s", pfnSecondaryColor3s, PFNGLSECONDARYCOLOR3SPROC);
    pfnSecondaryColor3s(red, green, blue);
}

void glSecondaryColor3sv(const GLshort *v)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3SVPROC) (const GLshort *v);
    static PFNGLSECONDARYCOLOR3SVPROC pfnSecondaryColor3sv = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3sv", pfnSecondaryColor3sv, PFNGLSECONDARYCOLOR3SVPROC);
    pfnSecondaryColor3sv(v);
}

void glSecondaryColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3UBPROC) (GLubyte red, GLubyte green, GLubyte blue);
    static PFNGLSECONDARYCOLOR3UBPROC pfnSecondaryColor3ub = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3ub", pfnSecondaryColor3ub, PFNGLSECONDARYCOLOR3UBPROC);
    pfnSecondaryColor3ub(red, green, blue);
}

void glSecondaryColor3ubv(const GLubyte *v)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3UBVPROC) (const GLubyte *v);
    static PFNGLSECONDARYCOLOR3UBVPROC pfnSecondaryColor3ubv = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3ubv", pfnSecondaryColor3ubv, PFNGLSECONDARYCOLOR3UBVPROC);
    pfnSecondaryColor3ubv(v);
}

void glSecondaryColor3ui(GLuint red, GLuint green, GLuint blue)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3UIPROC) (GLuint red, GLuint green, GLuint blue);
    static PFNGLSECONDARYCOLOR3UIPROC pfnSecondaryColor3ui = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3ui", pfnSecondaryColor3ui, PFNGLSECONDARYCOLOR3UIPROC);
    pfnSecondaryColor3ui(red, green, blue);
}

void glSecondaryColor3uiv(const GLuint *v)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3UIVPROC) (const GLuint *v);
    static PFNGLSECONDARYCOLOR3UIVPROC pfnSecondaryColor3uiv = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3uiv", pfnSecondaryColor3uiv, PFNGLSECONDARYCOLOR3UIVPROC);
    pfnSecondaryColor3uiv(v);
}

void glSecondaryColor3us(GLushort red, GLushort green, GLushort blue)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3USPROC) (GLushort red, GLushort green, GLushort blue);
    static PFNGLSECONDARYCOLOR3USPROC pfnSecondaryColor3us = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3us", pfnSecondaryColor3us, PFNGLSECONDARYCOLOR3USPROC);
    pfnSecondaryColor3us(red, green, blue);
}

void glSecondaryColor3usv(const GLushort *v)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLOR3USVPROC) (const GLushort *v);
    static PFNGLSECONDARYCOLOR3USVPROC pfnSecondaryColor3usv = 0;
    LOAD_ENTRYPOINT("glSecondaryColor3usv", pfnSecondaryColor3usv, PFNGLSECONDARYCOLOR3USVPROC);
    pfnSecondaryColor3usv(v);
}

void glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    typedef void (APIENTRY * PFNGLSECONDARYCOLORPOINTERPROC) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    static PFNGLSECONDARYCOLORPOINTERPROC pfnSecondaryColorPointer = 0;
    LOAD_ENTRYPOINT("glSecondaryColorPointer", pfnSecondaryColorPointer, PFNGLSECONDARYCOLORPOINTERPROC);
    pfnSecondaryColorPointer(size, type, stride, pointer);
}

void glWindowPos2d(GLdouble x, GLdouble y)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS2DPROC) (GLdouble x, GLdouble y);
    static PFNGLWINDOWPOS2DPROC pfnWindowPos2d = 0;
    LOAD_ENTRYPOINT("glWindowPos2d", pfnWindowPos2d, PFNGLWINDOWPOS2DPROC);
    pfnWindowPos2d(x, y);
}

void glWindowPos2dv(const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS2DVPROC) (const GLdouble *v);
    static PFNGLWINDOWPOS2DVPROC pfnWindowPos2dv = 0;
    LOAD_ENTRYPOINT("glWindowPos2dv", pfnWindowPos2dv, PFNGLWINDOWPOS2DVPROC);
    pfnWindowPos2dv(v);
}

void glWindowPos2f(GLfloat x, GLfloat y)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS2FPROC) (GLfloat x, GLfloat y);
    static PFNGLWINDOWPOS2FPROC pfnWindowPos2f = 0;
    LOAD_ENTRYPOINT("glWindowPos2f", pfnWindowPos2f, PFNGLWINDOWPOS2FPROC);
    pfnWindowPos2f(x, y);
}

void glWindowPos2fv(const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS2FVPROC) (const GLfloat *v);
    static PFNGLWINDOWPOS2FVPROC pfnWindowPos2fv = 0;
    LOAD_ENTRYPOINT("glWindowPos2fv", pfnWindowPos2fv, PFNGLWINDOWPOS2FVPROC);
    pfnWindowPos2fv(v);
}

void glWindowPos2i(GLint x, GLint y)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS2IPROC) (GLint x, GLint y);
    static PFNGLWINDOWPOS2IPROC pfnWindowPos2i = 0;
    LOAD_ENTRYPOINT("glWindowPos2i", pfnWindowPos2i, PFNGLWINDOWPOS2IPROC);
    pfnWindowPos2i(x, y);
}

void glWindowPos2iv(const GLint *v)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS2IVPROC) (const GLint *v);
    static PFNGLWINDOWPOS2IVPROC pfnWindowPos2iv = 0;
    LOAD_ENTRYPOINT("glWindowPos2iv", pfnWindowPos2iv, PFNGLWINDOWPOS2IVPROC);
    pfnWindowPos2iv(v);
}

void glWindowPos2s(GLshort x, GLshort y)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS2SPROC) (GLshort x, GLshort y);
    static PFNGLWINDOWPOS2SPROC pfnWindowPos2s = 0;
    LOAD_ENTRYPOINT("glWindowPos2s", pfnWindowPos2s, PFNGLWINDOWPOS2SPROC);
    pfnWindowPos2s(x, y);
}

void glWindowPos2sv(const GLshort *v)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS2SVPROC) (const GLshort *v);
    static PFNGLWINDOWPOS2SVPROC pfnWindowPos2sv = 0;
    LOAD_ENTRYPOINT("glWindowPos2sv", pfnWindowPos2sv, PFNGLWINDOWPOS2SVPROC);
    pfnWindowPos2sv(v);
}

void glWindowPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS3DPROC) (GLdouble x, GLdouble y, GLdouble z);
    static PFNGLWINDOWPOS3DPROC pfnWindowPos3d = 0;
    LOAD_ENTRYPOINT("glWindowPos3d", pfnWindowPos3d, PFNGLWINDOWPOS3DPROC);
    pfnWindowPos3d(x, y, z);
}

void glWindowPos3dv(const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS3DVPROC) (const GLdouble *v);
    static PFNGLWINDOWPOS3DVPROC pfnWindowPos3dv = 0;
    LOAD_ENTRYPOINT("glWindowPos3dv", pfnWindowPos3dv, PFNGLWINDOWPOS3DVPROC);
    pfnWindowPos3dv(v);
}

void glWindowPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS3FPROC) (GLfloat x, GLfloat y, GLfloat z);
    static PFNGLWINDOWPOS3FPROC pfnWindowPos3f = 0;
    LOAD_ENTRYPOINT("glWindowPos3f", pfnWindowPos3f, PFNGLWINDOWPOS3FPROC);
    pfnWindowPos3f(x, y, z);
}

void glWindowPos3fv(const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS3FVPROC) (const GLfloat *v);
    static PFNGLWINDOWPOS3FVPROC pfnWindowPos3fv = 0;
    LOAD_ENTRYPOINT("glWindowPos3fv", pfnWindowPos3fv, PFNGLWINDOWPOS3FVPROC);
    pfnWindowPos3fv(v);
}

void glWindowPos3i(GLint x, GLint y, GLint z)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS3IPROC) (GLint x, GLint y, GLint z);
    static PFNGLWINDOWPOS3IPROC pfnWindowPos3i = 0;
    LOAD_ENTRYPOINT("glWindowPos3i", pfnWindowPos3i, PFNGLWINDOWPOS3IPROC);
    pfnWindowPos3i(x, y, z);
}

void glWindowPos3iv(const GLint *v)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS3IVPROC) (const GLint *v);
    static PFNGLWINDOWPOS3IVPROC pfnWindowPos3iv = 0;
    LOAD_ENTRYPOINT("glWindowPos3iv", pfnWindowPos3iv, PFNGLWINDOWPOS3IVPROC);
    pfnWindowPos3iv(v);
}

void glWindowPos3s(GLshort x, GLshort y, GLshort z)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS3SPROC) (GLshort x, GLshort y, GLshort z);
    static PFNGLWINDOWPOS3SPROC pfnWindowPos3s = 0;
    LOAD_ENTRYPOINT("glWindowPos3s", pfnWindowPos3s, PFNGLWINDOWPOS3SPROC);
    pfnWindowPos3s(x, y, z);
}

void glWindowPos3sv(const GLshort *v)
{
    typedef void (APIENTRY * PFNGLWINDOWPOS3SVPROC) (const GLshort *v);
    static PFNGLWINDOWPOS3SVPROC pfnWindowPos3sv = 0;
    LOAD_ENTRYPOINT("glWindowPos3sv", pfnWindowPos3sv, PFNGLWINDOWPOS3SVPROC);
    pfnWindowPos3sv(v);
}

//
// OpenGL 1.5
//

void glGenQueries(GLsizei n, GLuint *ids)
{
    typedef void (APIENTRY * PFNGLGENQUERIESPROC) (GLsizei n, GLuint *ids);
    static PFNGLGENQUERIESPROC pfnGenQueries = 0;
    LOAD_ENTRYPOINT("glGenQueries", pfnGenQueries, PFNGLGENQUERIESPROC);
    pfnGenQueries(n, ids);
}

void glDeleteQueries(GLsizei n, const GLuint *ids)
{
    typedef void (APIENTRY * PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint *ids);
    static PFNGLDELETEQUERIESPROC pfnDeleteQueries = 0;
    LOAD_ENTRYPOINT("glDeleteQueries", pfnDeleteQueries, PFNGLDELETEQUERIESPROC);
    pfnDeleteQueries(n, ids);
}

GLboolean glIsQuery(GLuint id)
{
    typedef GLboolean (APIENTRY * PFNGLISQUERYPROC) (GLuint id);
    static PFNGLISQUERYPROC pfnIsQuery = 0;
    LOAD_ENTRYPOINT("glIsQuery", pfnIsQuery, PFNGLISQUERYPROC);
    return pfnIsQuery(id);
}

void glBeginQuery(GLenum target, GLuint id)
{
    typedef void (APIENTRY * PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
    static PFNGLBEGINQUERYPROC pfnBeginQuery = 0;
    LOAD_ENTRYPOINT("glBeginQuery", pfnBeginQuery, PFNGLBEGINQUERYPROC);
    pfnBeginQuery(target, id);
}

void glEndQuery(GLenum target)
{
    typedef void (APIENTRY * PFNGLENDQUERYPROC) (GLenum target);
    static PFNGLENDQUERYPROC pfnEndQuery = 0;
    LOAD_ENTRYPOINT("glEndQuery", pfnEndQuery, PFNGLENDQUERYPROC);
    pfnEndQuery(target);
}

void glGetQueryiv(GLenum target, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETQUERYIVPROC) (GLenum target, GLenum pname, GLint *params);
    static PFNGLGETQUERYIVPROC pfnGetQueryiv = 0;
    LOAD_ENTRYPOINT("glGetQueryiv", pfnGetQueryiv, PFNGLGETQUERYIVPROC);
    pfnGetQueryiv(target, pname, params);
}

void glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETQUERYOBJECTIVPROC) (GLuint id, GLenum pname, GLint *params);
    static PFNGLGETQUERYOBJECTIVPROC pfnGetQueryObjectiv = 0;
    LOAD_ENTRYPOINT("glGetQueryObjectiv", pfnGetQueryObjectiv, PFNGLGETQUERYOBJECTIVPROC);
    pfnGetQueryObjectiv(id, pname, params);
}

void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params)
{
    typedef void (APIENTRY * PFNGLGETQUERYOBJECTUIVPROC) (GLuint id, GLenum pname, GLuint *params);
    static PFNGLGETQUERYOBJECTUIVPROC pfnGetQueryObjectuiv = 0;
    LOAD_ENTRYPOINT("glGetQueryObjectuiv", pfnGetQueryObjectuiv, PFNGLGETQUERYOBJECTUIVPROC);
    pfnGetQueryObjectuiv(id, pname, params);
}

void glBindBuffer(GLenum target, GLuint buffer)
{
    typedef void (APIENTRY * PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
    static PFNGLBINDBUFFERPROC pfnBindBuffer = 0;
    LOAD_ENTRYPOINT("glBindBuffer", pfnBindBuffer, PFNGLBINDBUFFERPROC);
    pfnBindBuffer(target, buffer);
}

void glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
    typedef void (APIENTRY * PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
    static PFNGLDELETEBUFFERSPROC pfnDeleteBuffers = 0;
    LOAD_ENTRYPOINT("glDeleteBuffers", pfnDeleteBuffers, PFNGLDELETEBUFFERSPROC);
    pfnDeleteBuffers(n, buffers);
}

void glGenBuffers(GLsizei n, GLuint *buffers)
{
    typedef void (APIENTRY * PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
    static PFNGLGENBUFFERSPROC pfnGenBuffers = 0;
    LOAD_ENTRYPOINT("glGenBuffers", pfnGenBuffers, PFNGLGENBUFFERSPROC);
    pfnGenBuffers(n, buffers);
}

GLboolean glIsBuffer(GLuint buffer)
{
    typedef GLboolean (APIENTRY * PFNGLISBUFFERPROC) (GLuint buffer);
    static PFNGLISBUFFERPROC pfnIsBuffer = 0;
    LOAD_ENTRYPOINT("glIsBuffer", pfnIsBuffer, PFNGLISBUFFERPROC);
    return pfnIsBuffer(buffer);
}

void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
    typedef void (APIENTRY * PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
    static PFNGLBUFFERDATAPROC pfnBufferData = 0;
    LOAD_ENTRYPOINT("glBufferData", pfnBufferData, PFNGLBUFFERDATAPROC);
    pfnBufferData(target, size, data, usage);
}

void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
    static PFNGLBUFFERSUBDATAPROC pfnBufferSubData = 0;
    LOAD_ENTRYPOINT("glBufferSubData", pfnBufferSubData, PFNGLBUFFERSUBDATAPROC);
    pfnBufferSubData(target, offset, size, data);
}

void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data)
{
    typedef void (APIENTRY * PFNGLGETBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
    static PFNGLGETBUFFERSUBDATAPROC pfnGetBufferSubData = 0;
    LOAD_ENTRYPOINT("glGetBufferSubData", pfnGetBufferSubData, PFNGLGETBUFFERSUBDATAPROC);
    pfnGetBufferSubData(target, offset, size, data);
}

GLvoid* glMapBuffer(GLenum target, GLenum access)
{
    typedef GLvoid* (APIENTRY * PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
    static PFNGLMAPBUFFERPROC pfnMapBuffer = 0;
    LOAD_ENTRYPOINT("glMapBuffer", pfnMapBuffer, PFNGLMAPBUFFERPROC);
    return pfnMapBuffer(target, access);
}

GLboolean glUnmapBuffer(GLenum target)
{
    typedef GLboolean (APIENTRY * PFNGLUNMAPBUFFERPROC) (GLenum target);
    static PFNGLUNMAPBUFFERPROC pfnUnmapBuffer = 0;
    LOAD_ENTRYPOINT("glUnmapBuffer", pfnUnmapBuffer, PFNGLUNMAPBUFFERPROC);
    return pfnUnmapBuffer(target);
}

void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
    static PFNGLGETBUFFERPARAMETERIVPROC pfnGetBufferParameteriv = 0;
    LOAD_ENTRYPOINT("glGetBufferParameteriv", pfnGetBufferParameteriv, PFNGLGETBUFFERPARAMETERIVPROC);
    pfnGetBufferParameteriv(target, pname, params);
}

void glGetBufferPointerv(GLenum target, GLenum pname, GLvoid* *params)
{
    typedef void (APIENTRY * PFNGLGETBUFFERPOINTERVPROC) (GLenum target, GLenum pname, GLvoid* *params);
    static PFNGLGETBUFFERPOINTERVPROC pfnGetBufferPointerv = 0;
    LOAD_ENTRYPOINT("glGetBufferPointerv", pfnGetBufferPointerv, PFNGLGETBUFFERPOINTERVPROC);
    pfnGetBufferPointerv(target, pname, params);
}

//
// OpenGL 2.0
//

void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    typedef void (APIENTRY * PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum modeRGB, GLenum modeAlpha);
    static PFNGLBLENDEQUATIONSEPARATEPROC pfnBlendEquationSeparate = 0;
    LOAD_ENTRYPOINT("glBlendEquationSeparate", pfnBlendEquationSeparate, PFNGLBLENDEQUATIONSEPARATEPROC);
    pfnBlendEquationSeparate(modeRGB, modeAlpha);
}

void glDrawBuffers(GLsizei n, const GLenum *bufs)
{
    typedef void (APIENTRY * PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum *bufs);
    static PFNGLDRAWBUFFERSPROC pfnDrawBuffers = 0;
    LOAD_ENTRYPOINT("glDrawBuffers", pfnDrawBuffers, PFNGLDRAWBUFFERSPROC);
    pfnDrawBuffers(n, bufs);
}

void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
    typedef void (APIENTRY * PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
    static PFNGLSTENCILOPSEPARATEPROC pfnStencilOpSeparate = 0;
    LOAD_ENTRYPOINT("glStencilOpSeparate", pfnStencilOpSeparate, PFNGLSTENCILOPSEPARATEPROC);
    pfnStencilOpSeparate(face, sfail, dpfail, dppass);
}

void glStencilFuncSeparate(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask)
{
    typedef void (APIENTRY * PFNGLSTENCILFUNCSEPARATEPROC) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
    static PFNGLSTENCILFUNCSEPARATEPROC pfnStencilFuncSeparate = 0;
    LOAD_ENTRYPOINT("glStencilFuncSeparate", pfnStencilFuncSeparate, PFNGLSTENCILFUNCSEPARATEPROC);
    pfnStencilFuncSeparate(frontfunc, backfunc, ref, mask);
}

void glStencilMaskSeparate(GLenum face, GLuint mask)
{
    typedef void (APIENTRY * PFNGLSTENCILMASKSEPARATEPROC) (GLenum face, GLuint mask);
    static PFNGLSTENCILMASKSEPARATEPROC pfnStencilMaskSeparate = 0;
    LOAD_ENTRYPOINT("glStencilMaskSeparate", pfnStencilMaskSeparate, PFNGLSTENCILMASKSEPARATEPROC);
    pfnStencilMaskSeparate(face, mask);
}

void glAttachShader(GLuint program, GLuint shader)
{
    typedef void (APIENTRY * PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
    static PFNGLATTACHSHADERPROC pfnAttachShader = 0;
    LOAD_ENTRYPOINT("glAttachShader", pfnAttachShader, PFNGLATTACHSHADERPROC);
    pfnAttachShader(program, shader);
}

void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name)
{
    typedef void (APIENTRY * PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar *name);
    static PFNGLBINDATTRIBLOCATIONPROC pfnBindAttribLocation = 0;
    LOAD_ENTRYPOINT("glBindAttribLocation", pfnBindAttribLocation, PFNGLBINDATTRIBLOCATIONPROC);
    pfnBindAttribLocation(program, index, name);
}

void glCompileShader(GLuint shader)
{
    typedef void (APIENTRY * PFNGLCOMPILESHADERPROC) (GLuint shader);
    static PFNGLCOMPILESHADERPROC pfnCompileShader = 0;
    LOAD_ENTRYPOINT("glCompileShader", pfnCompileShader, PFNGLCOMPILESHADERPROC);
    pfnCompileShader(shader);
}

GLuint glCreateProgram()
{
    typedef GLuint (APIENTRY * PFNGLCREATEPROGRAMPROC) ();
    static PFNGLCREATEPROGRAMPROC pfnCreateProgram = 0;
    LOAD_ENTRYPOINT("glCreateProgram", pfnCreateProgram, PFNGLCREATEPROGRAMPROC);
    return pfnCreateProgram();
}

GLuint glCreateShader(GLenum type)
{
    typedef GLuint (APIENTRY * PFNGLCREATESHADERPROC) (GLenum type);
    static PFNGLCREATESHADERPROC pfnCreateShader = 0;
    LOAD_ENTRYPOINT("glCreateShader", pfnCreateShader, PFNGLCREATESHADERPROC);
    return pfnCreateShader(type);
}

void glDeleteProgram(GLuint program)
{
    typedef void (APIENTRY * PFNGLDELETEPROGRAMPROC) (GLuint program);
    static PFNGLDELETEPROGRAMPROC pfnDeleteProgram = 0;
    LOAD_ENTRYPOINT("glDeleteProgram", pfnDeleteProgram, PFNGLDELETEPROGRAMPROC);
    pfnDeleteProgram(program);
}

void glDeleteShader(GLuint shader)
{
    typedef void (APIENTRY * PFNGLDELETESHADERPROC) (GLuint shader);
    static PFNGLDELETESHADERPROC pfnDeleteShader = 0;
    LOAD_ENTRYPOINT("glDeleteShader", pfnDeleteShader, PFNGLDELETESHADERPROC);
    pfnDeleteShader(shader);
}

void glDetachShader(GLuint program, GLuint shader)
{
    typedef void (APIENTRY * PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
    static PFNGLDETACHSHADERPROC pfnDetachShader = 0;
    LOAD_ENTRYPOINT("glDetachShader", pfnDetachShader, PFNGLDETACHSHADERPROC);
    pfnDetachShader(program, shader);
}

void glDisableVertexAttribArray(GLuint index)
{
    typedef void (APIENTRY * PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
    static PFNGLDISABLEVERTEXATTRIBARRAYPROC pfnDisableVertexAttribArray = 0;
    LOAD_ENTRYPOINT("glDisableVertexAttribArray", pfnDisableVertexAttribArray, PFNGLDISABLEVERTEXATTRIBARRAYPROC);
    pfnDisableVertexAttribArray(index);
}

void glEnableVertexAttribArray(GLuint index)
{
    typedef void (APIENTRY * PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
    static PFNGLENABLEVERTEXATTRIBARRAYPROC pfnEnableVertexAttribArray = 0;
    LOAD_ENTRYPOINT("glEnableVertexAttribArray", pfnEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC);
    pfnEnableVertexAttribArray(index);
}

void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    typedef void (APIENTRY * PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
    static PFNGLGETACTIVEATTRIBPROC pfnGetActiveAttrib = 0;
    LOAD_ENTRYPOINT("glGetActiveAttrib", pfnGetActiveAttrib, PFNGLGETACTIVEATTRIBPROC);
    pfnGetActiveAttrib(program, index, bufSize, length, size, type, name);
}

void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
    static PFNGLGETACTIVEUNIFORMPROC pfnGetActiveUniform = 0;
    LOAD_ENTRYPOINT("glGetActiveUniform", pfnGetActiveUniform, PFNGLGETACTIVEUNIFORMPROC);
    pfnGetActiveUniform(program, index, bufSize, length, size, type, name);
}

void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj)
{
    typedef void (APIENTRY * PFNGLGETATTACHEDSHADERSPROC) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj);
    static PFNGLGETATTACHEDSHADERSPROC pfnGetAttachedShaders = 0;
    LOAD_ENTRYPOINT("glGetAttachedShaders", pfnGetAttachedShaders, PFNGLGETATTACHEDSHADERSPROC);
    pfnGetAttachedShaders(program, maxCount, count, obj);
}

GLint glGetAttribLocation(GLuint program, const GLchar *name)
{
    typedef GLint (APIENTRY * PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar *name);
    static PFNGLGETATTRIBLOCATIONPROC pfnGetAttribLocation = 0;
    LOAD_ENTRYPOINT("glGetAttribLocation", pfnGetAttribLocation, PFNGLGETATTRIBLOCATIONPROC);
    return pfnGetAttribLocation(program, name);
}

void glGetProgramiv(GLuint program, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
    static PFNGLGETPROGRAMIVPROC pfnGetProgramiv = 0;
    LOAD_ENTRYPOINT("glGetProgramiv", pfnGetProgramiv, PFNGLGETPROGRAMIVPROC);
    pfnGetProgramiv(program, pname, params);
}

void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    typedef void (APIENTRY * PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    static PFNGLGETPROGRAMINFOLOGPROC pfnGetProgramInfoLog = 0;
    LOAD_ENTRYPOINT("glGetProgramInfoLog", pfnGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC);
    pfnGetProgramInfoLog(program, bufSize, length, infoLog);
}

void glGetShaderiv(GLuint shader, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
    static PFNGLGETSHADERIVPROC pfnGetShaderiv = 0;
    LOAD_ENTRYPOINT("glGetShaderiv", pfnGetShaderiv, PFNGLGETSHADERIVPROC);
    pfnGetShaderiv(shader, pname, params);
}

void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    typedef void (APIENTRY * PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    static PFNGLGETSHADERINFOLOGPROC pfnGetShaderInfoLog = 0;
    LOAD_ENTRYPOINT("glGetShaderInfoLog", pfnGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC);
    pfnGetShaderInfoLog(shader, bufSize, length, infoLog);
}

void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)
{
    typedef void (APIENTRY * PFNGLGETSHADERSOURCEPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
    static PFNGLGETSHADERSOURCEPROC pfnGetShaderSource = 0;
    LOAD_ENTRYPOINT("glGetShaderSource", pfnGetShaderSource, PFNGLGETSHADERSOURCEPROC);
    pfnGetShaderSource(shader, bufSize, length, source);
}

GLint glGetUniformLocation(GLuint program, const GLchar *name)
{
    typedef GLint (APIENTRY * PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
    static PFNGLGETUNIFORMLOCATIONPROC pfnGetUniformLocation = 0;
    LOAD_ENTRYPOINT("glGetUniformLocation", pfnGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC);
    return pfnGetUniformLocation(program, name);
}

void glGetUniformfv(GLuint program, GLint location, GLfloat *params)
{
    typedef void (APIENTRY * PFNGLGETUNIFORMFVPROC) (GLuint program, GLint location, GLfloat *params);
    static PFNGLGETUNIFORMFVPROC pfnGetUniformfv = 0;
    LOAD_ENTRYPOINT("glGetUniformfv", pfnGetUniformfv, PFNGLGETUNIFORMFVPROC);
    pfnGetUniformfv(program, location, params);
}

void glGetUniformiv(GLuint program, GLint location, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETUNIFORMIVPROC) (GLuint program, GLint location, GLint *params);
    static PFNGLGETUNIFORMIVPROC pfnGetUniformiv = 0;
    LOAD_ENTRYPOINT("glGetUniformiv", pfnGetUniformiv, PFNGLGETUNIFORMIVPROC);
    pfnGetUniformiv(program, location, params);
}

void glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble *params)
{
    typedef void (APIENTRY * PFNGLGETVERTEXATTRIBDVPROC) (GLuint index, GLenum pname, GLdouble *params);
    static PFNGLGETVERTEXATTRIBDVPROC pfnGetVertexAttribdv = 0;
    LOAD_ENTRYPOINT("glGetVertexAttribdv", pfnGetVertexAttribdv, PFNGLGETVERTEXATTRIBDVPROC);
    pfnGetVertexAttribdv(index, pname, params);
}

void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
    typedef void (APIENTRY * PFNGLGETVERTEXATTRIBFVPROC) (GLuint index, GLenum pname, GLfloat *params);
    static PFNGLGETVERTEXATTRIBFVPROC pfnGetVertexAttribfv = 0;
    LOAD_ENTRYPOINT("glGetVertexAttribfv", pfnGetVertexAttribfv, PFNGLGETVERTEXATTRIBFVPROC);
    pfnGetVertexAttribfv(index, pname, params);
}

void glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETVERTEXATTRIBIVPROC) (GLuint index, GLenum pname, GLint *params);
    static PFNGLGETVERTEXATTRIBIVPROC pfnGetVertexAttribiv = 0;
    LOAD_ENTRYPOINT("glGetVertexAttribiv", pfnGetVertexAttribiv, PFNGLGETVERTEXATTRIBIVPROC);
    pfnGetVertexAttribiv(index, pname, params);
}

void glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid* *pointer)
{
    typedef void (APIENTRY * PFNGLGETVERTEXATTRIBPOINTERVPROC) (GLuint index, GLenum pname, GLvoid* *pointer);
    static PFNGLGETVERTEXATTRIBPOINTERVPROC pfnGetVertexAttribPointerv = 0;
    LOAD_ENTRYPOINT("glGetVertexAttribPointerv", pfnGetVertexAttribPointerv, PFNGLGETVERTEXATTRIBPOINTERVPROC);
    pfnGetVertexAttribPointerv(index, pname, pointer);
}

GLboolean glIsProgram(GLuint program)
{
    typedef GLboolean (APIENTRY * PFNGLISPROGRAMPROC) (GLuint program);
    static PFNGLISPROGRAMPROC pfnIsProgram = 0;
    LOAD_ENTRYPOINT("glIsProgram", pfnIsProgram, PFNGLISPROGRAMPROC);
    return pfnIsProgram(program);
}

GLboolean glIsShader(GLuint shader)
{
    typedef GLboolean (APIENTRY * PFNGLISSHADERPROC) (GLuint shader);
    static PFNGLISSHADERPROC pfnIsShader = 0;
    LOAD_ENTRYPOINT("glIsShader", pfnIsShader, PFNGLISSHADERPROC);
    return pfnIsShader(shader);
}

void glLinkProgram(GLuint program)
{
    typedef void (APIENTRY * PFNGLLINKPROGRAMPROC) (GLuint program);
    static PFNGLLINKPROGRAMPROC pfnLinkProgram = 0;
    LOAD_ENTRYPOINT("glLinkProgram", pfnLinkProgram, PFNGLLINKPROGRAMPROC);
    pfnLinkProgram(program);
}

void glShaderSource(GLuint shader, GLsizei count, const GLchar* *string, const GLint *length)
{
    typedef void (APIENTRY * PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
    static PFNGLSHADERSOURCEPROC pfnShaderSource = 0;
    LOAD_ENTRYPOINT("glShaderSource", pfnShaderSource, PFNGLSHADERSOURCEPROC);
    pfnShaderSource(shader, count, string, length);
}

void glUseProgram(GLuint program)
{
    typedef void (APIENTRY * PFNGLUSEPROGRAMPROC) (GLuint program);
    static PFNGLUSEPROGRAMPROC pfnUseProgram = 0;
    LOAD_ENTRYPOINT("glUseProgram", pfnUseProgram, PFNGLUSEPROGRAMPROC);
    pfnUseProgram(program);
}

void glUniform1f(GLint location, GLfloat v0)
{
    typedef void (APIENTRY * PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
    static PFNGLUNIFORM1FPROC pfnUniform1f = 0;
    LOAD_ENTRYPOINT("glUniform1f", pfnUniform1f, PFNGLUNIFORM1FPROC);
    pfnUniform1f(location, v0);
}

void glUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
    typedef void (APIENTRY * PFNGLUNIFORM2FPROC) (GLint location, GLfloat v0, GLfloat v1);
    static PFNGLUNIFORM2FPROC pfnUniform2f = 0;
    LOAD_ENTRYPOINT("glUniform2f", pfnUniform2f, PFNGLUNIFORM2FPROC);
    pfnUniform2f(location, v0, v1);
}

void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    typedef void (APIENTRY * PFNGLUNIFORM3FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    static PFNGLUNIFORM3FPROC pfnUniform3f = 0;
    LOAD_ENTRYPOINT("glUniform3f", pfnUniform3f, PFNGLUNIFORM3FPROC);
    pfnUniform3f(location, v0, v1, v2);
}

void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    typedef void (APIENTRY * PFNGLUNIFORM4FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    static PFNGLUNIFORM4FPROC pfnUniform4f = 0;
    LOAD_ENTRYPOINT("glUniform4f", pfnUniform4f, PFNGLUNIFORM4FPROC);
    pfnUniform4f(location, v0, v1, v2, v3);
}

void glUniform1i(GLint location, GLint v0)
{
    typedef void (APIENTRY * PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
    static PFNGLUNIFORM1IPROC pfnUniform1i = 0;
    LOAD_ENTRYPOINT("glUniform1i", pfnUniform1i, PFNGLUNIFORM1IPROC);
    pfnUniform1i(location, v0);
}

void glUniform2i(GLint location, GLint v0, GLint v1)
{
    typedef void (APIENTRY * PFNGLUNIFORM2IPROC) (GLint location, GLint v0, GLint v1);
    static PFNGLUNIFORM2IPROC pfnUniform2i = 0;
    LOAD_ENTRYPOINT("glUniform2i", pfnUniform2i, PFNGLUNIFORM2IPROC);
    pfnUniform2i(location, v0, v1);
}

void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
    typedef void (APIENTRY * PFNGLUNIFORM3IPROC) (GLint location, GLint v0, GLint v1, GLint v2);
    static PFNGLUNIFORM3IPROC pfnUniform3i = 0;
    LOAD_ENTRYPOINT("glUniform3i", pfnUniform3i, PFNGLUNIFORM3IPROC);
    pfnUniform3i(location, v0, v1, v2);
}

void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    typedef void (APIENTRY * PFNGLUNIFORM4IPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    static PFNGLUNIFORM4IPROC pfnUniform4i = 0;
    LOAD_ENTRYPOINT("glUniform4i", pfnUniform4i, PFNGLUNIFORM4IPROC);
    pfnUniform4i(location, v0, v1, v2, v3);
}

void glUniform1fv(GLint location, GLsizei count, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat *value);
    static PFNGLUNIFORM1FVPROC pfnUniform1fv = 0;
    LOAD_ENTRYPOINT("glUniform1fv", pfnUniform1fv, PFNGLUNIFORM1FVPROC);
    pfnUniform1fv(location, count, value);
}

void glUniform2fv(GLint location, GLsizei count, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat *value);
    static PFNGLUNIFORM2FVPROC pfnUniform2fv = 0;
    LOAD_ENTRYPOINT("glUniform2fv", pfnUniform2fv, PFNGLUNIFORM2FVPROC);
    pfnUniform2fv(location, count, value);
}

void glUniform3fv(GLint location, GLsizei count, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat *value);
    static PFNGLUNIFORM3FVPROC pfnUniform3fv = 0;
    LOAD_ENTRYPOINT("glUniform3fv", pfnUniform3fv, PFNGLUNIFORM3FVPROC);
    pfnUniform3fv(location, count, value);
}

void glUniform4fv(GLint location, GLsizei count, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
    static PFNGLUNIFORM4FVPROC pfnUniform4fv = 0;
    LOAD_ENTRYPOINT("glUniform4fv", pfnUniform4fv, PFNGLUNIFORM4FVPROC);
    pfnUniform4fv(location, count, value);
}

void glUniform1iv(GLint location, GLsizei count, const GLint *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint *value);
    static PFNGLUNIFORM1IVPROC pfnUniform1iv = 0;
    LOAD_ENTRYPOINT("glUniform1iv", pfnUniform1iv, PFNGLUNIFORM1IVPROC);
    pfnUniform1iv(location, count, value);
}

void glUniform2iv(GLint location, GLsizei count, const GLint *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint *value);
    static PFNGLUNIFORM2IVPROC pfnUniform2iv = 0;
    LOAD_ENTRYPOINT("glUniform2iv", pfnUniform2iv, PFNGLUNIFORM2IVPROC);
    pfnUniform2iv(location, count, value);
}

void glUniform3iv(GLint location, GLsizei count, const GLint *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint *value);
    static PFNGLUNIFORM3IVPROC pfnUniform3iv = 0;
    LOAD_ENTRYPOINT("glUniform3iv", pfnUniform3iv, PFNGLUNIFORM3IVPROC);
    pfnUniform3iv(location, count, value);
}

void glUniform4iv(GLint location, GLsizei count, const GLint *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint *value);
    static PFNGLUNIFORM4IVPROC pfnUniform4iv = 0;
    LOAD_ENTRYPOINT("glUniform4iv", pfnUniform4iv, PFNGLUNIFORM4IVPROC);
    pfnUniform4iv(location, count, value);
}

void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX2FVPROC pfnUniformMatrix2fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix2fv", pfnUniformMatrix2fv, PFNGLUNIFORMMATRIX2FVPROC);
    pfnUniformMatrix2fv(location, count, transpose, value);
}

void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX3FVPROC pfnUniformMatrix3fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix3fv", pfnUniformMatrix3fv, PFNGLUNIFORMMATRIX3FVPROC);
    pfnUniformMatrix3fv(location, count, transpose, value);
}

void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX4FVPROC pfnUniformMatrix4fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix4fv", pfnUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC);
    pfnUniformMatrix4fv(location, count, transpose, value);
}

void glValidateProgram(GLuint program)
{
    typedef void (APIENTRY * PFNGLVALIDATEPROGRAMPROC) (GLuint program);
    static PFNGLVALIDATEPROGRAMPROC pfnValidateProgram = 0;
    LOAD_ENTRYPOINT("glValidateProgram", pfnValidateProgram, PFNGLVALIDATEPROGRAMPROC);
    pfnValidateProgram(program);
}

void glVertexAttrib1d(GLuint index, GLdouble x)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB1DPROC) (GLuint index, GLdouble x);
    static PFNGLVERTEXATTRIB1DPROC pfnVertexAttrib1d = 0;
    LOAD_ENTRYPOINT("glVertexAttrib1d", pfnVertexAttrib1d, PFNGLVERTEXATTRIB1DPROC);
    pfnVertexAttrib1d(index, x);
}

void glVertexAttrib1dv(GLuint index, const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB1DVPROC) (GLuint index, const GLdouble *v);
    static PFNGLVERTEXATTRIB1DVPROC pfnVertexAttrib1dv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib1dv", pfnVertexAttrib1dv, PFNGLVERTEXATTRIB1DVPROC);
    pfnVertexAttrib1dv(index, v);
}

void glVertexAttrib1f(GLuint index, GLfloat x)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB1FPROC) (GLuint index, GLfloat x);
    static PFNGLVERTEXATTRIB1FPROC pfnVertexAttrib1f = 0;
    LOAD_ENTRYPOINT("glVertexAttrib1f", pfnVertexAttrib1f, PFNGLVERTEXATTRIB1FPROC);
    pfnVertexAttrib1f(index, x);
}

void glVertexAttrib1fv(GLuint index, const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB1FVPROC) (GLuint index, const GLfloat *v);
    static PFNGLVERTEXATTRIB1FVPROC pfnVertexAttrib1fv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib1fv", pfnVertexAttrib1fv, PFNGLVERTEXATTRIB1FVPROC);
    pfnVertexAttrib1fv(index, v);
}

void glVertexAttrib1s(GLuint index, GLshort x)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB1SPROC) (GLuint index, GLshort x);
    static PFNGLVERTEXATTRIB1SPROC pfnVertexAttrib1s = 0;
    LOAD_ENTRYPOINT("glVertexAttrib1s", pfnVertexAttrib1s, PFNGLVERTEXATTRIB1SPROC);
    pfnVertexAttrib1s(index, x);
}

void glVertexAttrib1sv(GLuint index, const GLshort *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB1SVPROC) (GLuint index, const GLshort *v);
    static PFNGLVERTEXATTRIB1SVPROC pfnVertexAttrib1sv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib1sv", pfnVertexAttrib1sv, PFNGLVERTEXATTRIB1SVPROC);
    pfnVertexAttrib1sv(index, v);
}

void glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB2DPROC) (GLuint index, GLdouble x, GLdouble y);
    static PFNGLVERTEXATTRIB2DPROC pfnVertexAttrib2d = 0;
    LOAD_ENTRYPOINT("glVertexAttrib2d", pfnVertexAttrib2d, PFNGLVERTEXATTRIB2DPROC);
    pfnVertexAttrib2d(index, x, y);
}

void glVertexAttrib2dv(GLuint index, const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB2DVPROC) (GLuint index, const GLdouble *v);
    static PFNGLVERTEXATTRIB2DVPROC pfnVertexAttrib2dv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib2dv", pfnVertexAttrib2dv, PFNGLVERTEXATTRIB2DVPROC);
    pfnVertexAttrib2dv(index, v);
}

void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB2FPROC) (GLuint index, GLfloat x, GLfloat y);
    static PFNGLVERTEXATTRIB2FPROC pfnVertexAttrib2f = 0;
    LOAD_ENTRYPOINT("glVertexAttrib2f", pfnVertexAttrib2f, PFNGLVERTEXATTRIB2FPROC);
    pfnVertexAttrib2f(index, x, y);
}

void glVertexAttrib2fv(GLuint index, const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB2FVPROC) (GLuint index, const GLfloat *v);
    static PFNGLVERTEXATTRIB2FVPROC pfnVertexAttrib2fv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib2fv", pfnVertexAttrib2fv, PFNGLVERTEXATTRIB2FVPROC);
    pfnVertexAttrib2fv(index, v);
}

void glVertexAttrib2s(GLuint index, GLshort x, GLshort y)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB2SPROC) (GLuint index, GLshort x, GLshort y);
    static PFNGLVERTEXATTRIB2SPROC pfnVertexAttrib2s = 0;
    LOAD_ENTRYPOINT("glVertexAttrib2s", pfnVertexAttrib2s, PFNGLVERTEXATTRIB2SPROC);
    pfnVertexAttrib2s(index, x, y);
}

void glVertexAttrib2sv(GLuint index, const GLshort *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB2SVPROC) (GLuint index, const GLshort *v);
    static PFNGLVERTEXATTRIB2SVPROC pfnVertexAttrib2sv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib2sv", pfnVertexAttrib2sv, PFNGLVERTEXATTRIB2SVPROC);
    pfnVertexAttrib2sv(index, v);
}

void glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB3DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
    static PFNGLVERTEXATTRIB3DPROC pfnVertexAttrib3d = 0;
    LOAD_ENTRYPOINT("glVertexAttrib3d", pfnVertexAttrib3d, PFNGLVERTEXATTRIB3DPROC);
    pfnVertexAttrib3d(index, x, y, z);
}

void glVertexAttrib3dv(GLuint index, const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB3DVPROC) (GLuint index, const GLdouble *v);
    static PFNGLVERTEXATTRIB3DVPROC pfnVertexAttrib3dv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib3dv", pfnVertexAttrib3dv, PFNGLVERTEXATTRIB3DVPROC);
    pfnVertexAttrib3dv(index, v);
}

void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB3FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
    static PFNGLVERTEXATTRIB3FPROC pfnVertexAttrib3f = 0;
    LOAD_ENTRYPOINT("glVertexAttrib3f", pfnVertexAttrib3f, PFNGLVERTEXATTRIB3FPROC);
    pfnVertexAttrib3f(index, x, y, z);
}

void glVertexAttrib3fv(GLuint index, const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB3FVPROC) (GLuint index, const GLfloat *v);
    static PFNGLVERTEXATTRIB3FVPROC pfnVertexAttrib3fv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib3fv", pfnVertexAttrib3fv, PFNGLVERTEXATTRIB3FVPROC);
    pfnVertexAttrib3fv(index, v);
}

void glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB3SPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
    static PFNGLVERTEXATTRIB3SPROC pfnVertexAttrib3s = 0;
    LOAD_ENTRYPOINT("glVertexAttrib3s", pfnVertexAttrib3s, PFNGLVERTEXATTRIB3SPROC);
    pfnVertexAttrib3s(index, x, y, z);
}

void glVertexAttrib3sv(GLuint index, const GLshort *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB3SVPROC) (GLuint index, const GLshort *v);
    static PFNGLVERTEXATTRIB3SVPROC pfnVertexAttrib3sv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib3sv", pfnVertexAttrib3sv, PFNGLVERTEXATTRIB3SVPROC);
    pfnVertexAttrib3sv(index, v);
}

void glVertexAttrib4Nbv(GLuint index, const GLbyte *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4NBVPROC) (GLuint index, const GLbyte *v);
    static PFNGLVERTEXATTRIB4NBVPROC pfnVertexAttrib4Nbv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4Nbv", pfnVertexAttrib4Nbv, PFNGLVERTEXATTRIB4NBVPROC);
    pfnVertexAttrib4Nbv(index, v);
}

void glVertexAttrib4Niv(GLuint index, const GLint *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4NIVPROC) (GLuint index, const GLint *v);
    static PFNGLVERTEXATTRIB4NIVPROC pfnVertexAttrib4Niv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4Niv", pfnVertexAttrib4Niv, PFNGLVERTEXATTRIB4NIVPROC);
    pfnVertexAttrib4Niv(index, v);
}

void glVertexAttrib4Nsv(GLuint index, const GLshort *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4NSVPROC) (GLuint index, const GLshort *v);
    static PFNGLVERTEXATTRIB4NSVPROC pfnVertexAttrib4Nsv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4Nsv", pfnVertexAttrib4Nsv, PFNGLVERTEXATTRIB4NSVPROC);
    pfnVertexAttrib4Nsv(index, v);
}

void glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUBPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
    static PFNGLVERTEXATTRIB4NUBPROC pfnVertexAttrib4Nub = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4Nub", pfnVertexAttrib4Nub, PFNGLVERTEXATTRIB4NUBPROC);
    pfnVertexAttrib4Nub(index, x, y, z, w);
}

void glVertexAttrib4Nubv(GLuint index, const GLubyte *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUBVPROC) (GLuint index, const GLubyte *v);
    static PFNGLVERTEXATTRIB4NUBVPROC pfnVertexAttrib4Nubv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4Nubv", pfnVertexAttrib4Nubv, PFNGLVERTEXATTRIB4NUBVPROC);
    pfnVertexAttrib4Nubv(index, v);
}

void glVertexAttrib4Nuiv(GLuint index, const GLuint *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUIVPROC) (GLuint index, const GLuint *v);
    static PFNGLVERTEXATTRIB4NUIVPROC pfnVertexAttrib4Nuiv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4Nuiv", pfnVertexAttrib4Nuiv, PFNGLVERTEXATTRIB4NUIVPROC);
    pfnVertexAttrib4Nuiv(index, v);
}

void glVertexAttrib4Nusv(GLuint index, const GLushort *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUSVPROC) (GLuint index, const GLushort *v);
    static PFNGLVERTEXATTRIB4NUSVPROC pfnVertexAttrib4Nusv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4Nusv", pfnVertexAttrib4Nusv, PFNGLVERTEXATTRIB4NUSVPROC);
    pfnVertexAttrib4Nusv(index, v);
}

void glVertexAttrib4bv(GLuint index, const GLbyte *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4BVPROC) (GLuint index, const GLbyte *v);
    static PFNGLVERTEXATTRIB4BVPROC pfnVertexAttrib4bv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4bv", pfnVertexAttrib4bv, PFNGLVERTEXATTRIB4BVPROC);
    pfnVertexAttrib4bv(index, v);
}

void glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    static PFNGLVERTEXATTRIB4DPROC pfnVertexAttrib4d = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4d", pfnVertexAttrib4d, PFNGLVERTEXATTRIB4DPROC);
    pfnVertexAttrib4d(index, x, y, z, w);
}

void glVertexAttrib4dv(GLuint index, const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4DVPROC) (GLuint index, const GLdouble *v);
    static PFNGLVERTEXATTRIB4DVPROC pfnVertexAttrib4dv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4dv", pfnVertexAttrib4dv, PFNGLVERTEXATTRIB4DVPROC);
    pfnVertexAttrib4dv(index, v);
}

void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    static PFNGLVERTEXATTRIB4FPROC pfnVertexAttrib4f = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4f", pfnVertexAttrib4f, PFNGLVERTEXATTRIB4FPROC);
    pfnVertexAttrib4f(index, x, y, z, w);
}

void glVertexAttrib4fv(GLuint index, const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4FVPROC) (GLuint index, const GLfloat *v);
    static PFNGLVERTEXATTRIB4FVPROC pfnVertexAttrib4fv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4fv", pfnVertexAttrib4fv, PFNGLVERTEXATTRIB4FVPROC);
    pfnVertexAttrib4fv(index, v);
}

void glVertexAttrib4iv(GLuint index, const GLint *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4IVPROC) (GLuint index, const GLint *v);
    static PFNGLVERTEXATTRIB4IVPROC pfnVertexAttrib4iv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4iv", pfnVertexAttrib4iv, PFNGLVERTEXATTRIB4IVPROC);
    pfnVertexAttrib4iv(index, v);
}

void glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4SPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
    static PFNGLVERTEXATTRIB4SPROC pfnVertexAttrib4s = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4s", pfnVertexAttrib4s, PFNGLVERTEXATTRIB4SPROC);
    pfnVertexAttrib4s(index, x, y, z, w);
}

void glVertexAttrib4sv(GLuint index, const GLshort *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4SVPROC) (GLuint index, const GLshort *v);
    static PFNGLVERTEXATTRIB4SVPROC pfnVertexAttrib4sv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4sv", pfnVertexAttrib4sv, PFNGLVERTEXATTRIB4SVPROC);
    pfnVertexAttrib4sv(index, v);
}

void glVertexAttrib4ubv(GLuint index, const GLubyte *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4UBVPROC) (GLuint index, const GLubyte *v);
    static PFNGLVERTEXATTRIB4UBVPROC pfnVertexAttrib4ubv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4ubv", pfnVertexAttrib4ubv, PFNGLVERTEXATTRIB4UBVPROC);
    pfnVertexAttrib4ubv(index, v);
}

void glVertexAttrib4uiv(GLuint index, const GLuint *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4UIVPROC) (GLuint index, const GLuint *v);
    static PFNGLVERTEXATTRIB4UIVPROC pfnVertexAttrib4uiv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4uiv", pfnVertexAttrib4uiv, PFNGLVERTEXATTRIB4UIVPROC);
    pfnVertexAttrib4uiv(index, v);
}

void glVertexAttrib4usv(GLuint index, const GLushort *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4USVPROC) (GLuint index, const GLushort *v);
    static PFNGLVERTEXATTRIB4USVPROC pfnVertexAttrib4usv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4usv", pfnVertexAttrib4usv, PFNGLVERTEXATTRIB4USVPROC);
    pfnVertexAttrib4usv(index, v);
}

void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
    static PFNGLVERTEXATTRIBPOINTERPROC pfnVertexAttribPointer = 0;
    LOAD_ENTRYPOINT("glVertexAttribPointer", pfnVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC);
    pfnVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

//
// OpenGL 2.1
//

void glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX2X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX2X3FVPROC pfnUniformMatrix2x3fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix2x3fv", pfnUniformMatrix2x3fv, PFNGLUNIFORMMATRIX2X3FVPROC);
    pfnUniformMatrix2x3fv(location, count, transpose, value);
}

void glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX3X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX3X2FVPROC pfnUniformMatrix3x2fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix3x2fv", pfnUniformMatrix3x2fv, PFNGLUNIFORMMATRIX3X2FVPROC);
    pfnUniformMatrix3x2fv(location, count, transpose, value);
}

void glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX2X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX2X4FVPROC pfnUniformMatrix2x4fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix2x4fv", pfnUniformMatrix2x4fv, PFNGLUNIFORMMATRIX2X4FVPROC);
    pfnUniformMatrix2x4fv(location, count, transpose, value);
}

void glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX4X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX4X2FVPROC pfnUniformMatrix4x2fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix4x2fv", pfnUniformMatrix4x2fv, PFNGLUNIFORMMATRIX4X2FVPROC);
    pfnUniformMatrix4x2fv(location, count, transpose, value);
}

void glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX3X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX3X4FVPROC pfnUniformMatrix3x4fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix3x4fv", pfnUniformMatrix3x4fv, PFNGLUNIFORMMATRIX3X4FVPROC);
    pfnUniformMatrix3x4fv(location, count, transpose, value);
}

void glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX4X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX4X3FVPROC pfnUniformMatrix4x3fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix4x3fv", pfnUniformMatrix4x3fv, PFNGLUNIFORMMATRIX4X3FVPROC);
    pfnUniformMatrix4x3fv(location, count, transpose, value);
}

//
// OpenGL 3.0
//

void glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
	typedef void (APIENTRY * PFNGLCOLORMASKIPROC)(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
	static PFNGLCOLORMASKIPROC pfnColorMaski = 0;
	LOAD_ENTRYPOINT("glColorMaski", pfnColorMaski, PFNGLCOLORMASKIPROC);
	pfnColorMaski(index, r, g, b, a);
}

void glGetBooleani_v(GLenum target, GLuint index, GLboolean *data)
{
	typedef void (APIENTRY * PFNGLGETBOOLEANI_VPROC)(GLenum target, GLuint index, GLboolean *data);
	static PFNGLGETBOOLEANI_VPROC pfnGetBooleani_v = 0;
	LOAD_ENTRYPOINT("glGetBooleani_v", pfnGetBooleani_v, PFNGLGETBOOLEANI_VPROC);
	pfnGetBooleani_v(target, index, data);
}

void glGetIntegeri_v(GLenum target, GLuint index, GLint *data)
{
	typedef void (APIENTRY * PFNGLGETINTEGERI_VPROC)(GLenum target, GLuint index, GLint *data);
	static PFNGLGETINTEGERI_VPROC pfnGetIntegeri_v = 0;
	LOAD_ENTRYPOINT("glGetIntegeri_v", pfnGetIntegeri_v, PFNGLGETINTEGERI_VPROC);
	pfnGetIntegeri_v(target, index, data);
}

void glEnablei(GLenum target, GLuint index)
{
	typedef void (APIENTRY * PFNGLENABLEIPROC)(GLenum target, GLuint index);
	static PFNGLENABLEIPROC pfnEnablei = 0;
	LOAD_ENTRYPOINT("glEnablei", pfnEnablei, PFNGLENABLEIPROC);
	pfnEnablei(target, index);
}

void glDisablei(GLenum target, GLuint index)
{
	typedef void (APIENTRY * PFNGLDISABLEIPROC)(GLenum target, GLuint index);
	static PFNGLDISABLEIPROC pfnDisablei = 0;
	LOAD_ENTRYPOINT("glDisablei", pfnDisablei, PFNGLDISABLEIPROC);
	pfnDisablei(target, index);
}

GLboolean glIsEnabledi(GLenum target, GLuint index)
{
	typedef GLboolean (APIENTRY * PFNGLISENABLEDIPROC)(GLenum target, GLuint index);
	static PFNGLISENABLEDIPROC pfnIsEnabledi = 0;
	LOAD_ENTRYPOINT("glIsEnabledi", pfnIsEnabledi, PFNGLISENABLEDIPROC);
	return pfnIsEnabledi(target, index);
}

void glBeginTransformFeedback(GLenum primitiveMode)
{
	typedef void (APIENTRY * PFNGLBEGINTRANSFORMFEEDBACKPROC)(GLenum primitiveMode);
	static PFNGLBEGINTRANSFORMFEEDBACKPROC pfnBeginTransformFeedback = 0;
	LOAD_ENTRYPOINT("glBeginTransformFeedback", pfnBeginTransformFeedback, PFNGLBEGINTRANSFORMFEEDBACKPROC);
	pfnBeginTransformFeedback(primitiveMode);
}

void glEndTransformFeedback(void)
{
	typedef void (APIENTRY * PFNGLENDTRANSFORMFEEDBACKPROC)(void);
	static PFNGLENDTRANSFORMFEEDBACKPROC pfnEndTransformFeedback = 0;
	LOAD_ENTRYPOINT("glEndTransformFeedback", pfnEndTransformFeedback, PFNGLENDTRANSFORMFEEDBACKPROC);
	pfnEndTransformFeedback();
}

void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	typedef void (APIENTRY * PFNGLBINDBUFFERRANGEPROC)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static PFNGLBINDBUFFERRANGEPROC pfnBindBufferRange = 0;
	LOAD_ENTRYPOINT("glBindBufferRange", pfnBindBufferRange, PFNGLBINDBUFFERRANGEPROC);
	pfnBindBufferRange(target, index, buffer, offset, size);
}

void glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
	typedef void (APIENTRY * PFNGLBINDBUFFERBASEPROC)(GLenum target, GLuint index, GLuint buffer);
	static PFNGLBINDBUFFERBASEPROC pfnBindBufferBase = 0;
	LOAD_ENTRYPOINT("glBindBufferBase", pfnBindBufferBase, PFNGLBINDBUFFERBASEPROC);
	pfnBindBufferBase(target, index, buffer);
}

void glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar **varyings, GLenum bufferMode)
{
	typedef void (APIENTRY * PFNGLTRANSFORMFEEDBACKVARYINGSPROC)(GLuint program, GLsizei count, const GLchar **varyings, GLenum bufferMode);
	static PFNGLTRANSFORMFEEDBACKVARYINGSPROC pfnTransformFeedbackVaryings = 0;
	LOAD_ENTRYPOINT("glTransformFeedbackVaryings", pfnTransformFeedbackVaryings, PFNGLTRANSFORMFEEDBACKVARYINGSPROC);
	pfnTransformFeedbackVaryings(program, count, varyings, bufferMode);
}

void glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)
{
	typedef void (APIENTRY * PFNGLGETTRANSFORMFEEDBACKVARYINGPROC)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
	static PFNGLGETTRANSFORMFEEDBACKVARYINGPROC pfnGetTransformFeedbackVarying = 0;
	LOAD_ENTRYPOINT("glGetTransformFeedbackVarying", pfnGetTransformFeedbackVarying, PFNGLGETTRANSFORMFEEDBACKVARYINGPROC);
	pfnGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
}

void glClampColor(GLenum target, GLenum clamp)
{
	typedef void (APIENTRY * PFNGLCLAMPCOLORPROC)(GLenum target, GLenum clamp);
	static PFNGLCLAMPCOLORPROC pfnClampColor = 0;
	LOAD_ENTRYPOINT("glClampColor", pfnClampColor, PFNGLCLAMPCOLORPROC);
	pfnClampColor(target, clamp);
}

void glBeginConditionalRender(GLuint id, GLenum mode)
{
	typedef void (APIENTRY * PFNGLBEGINCONDITIONALRENDERPROC)(GLuint id, GLenum mode);
	static PFNGLBEGINCONDITIONALRENDERPROC pfnBeginConditionalRender = 0;
	LOAD_ENTRYPOINT("glBeginConditionalRender", pfnBeginConditionalRender, PFNGLBEGINCONDITIONALRENDERPROC);
	pfnBeginConditionalRender(id, mode);
}

void glEndConditionalRender(void)
{
	typedef void (APIENTRY * PFNGLENDCONDITIONALRENDERPROC)(void);
	static PFNGLENDCONDITIONALRENDERPROC pfnEndConditionalRender = 0;
	LOAD_ENTRYPOINT("glEndConditionalRender", pfnEndConditionalRender, PFNGLENDCONDITIONALRENDERPROC);
	pfnEndConditionalRender();
}

void glVertexAttribI1i(GLuint index, GLint x)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI1IPROC)(GLuint index, GLint x);
	static PFNGLVERTEXATTRIBI1IPROC pfnVertexAttribI1i = 0;
	LOAD_ENTRYPOINT("glVertexAttribI1i", pfnVertexAttribI1i, PFNGLVERTEXATTRIBI1IPROC);
	pfnVertexAttribI1i(index, x);
}

void glVertexAttribI2i(GLuint index, GLint x, GLint y)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI2IPROC)(GLuint index, GLint x, GLint y);
	static PFNGLVERTEXATTRIBI2IPROC pfnVertexAttribI2i = 0;
	LOAD_ENTRYPOINT("glVertexAttribI2i", pfnVertexAttribI2i, PFNGLVERTEXATTRIBI2IPROC);
	pfnVertexAttribI2i(index, x, y);
}

void glVertexAttribI3i(GLuint index, GLint x, GLint y, GLint z)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI3IPROC)(GLuint index, GLint x, GLint y, GLint z);
	static PFNGLVERTEXATTRIBI3IPROC pfnVertexAttribI3i = 0;
	LOAD_ENTRYPOINT("glVertexAttribI3i", pfnVertexAttribI3i, PFNGLVERTEXATTRIBI3IPROC);
	pfnVertexAttribI3i(index, x, y, z);
}

void glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4IPROC)(GLuint index, GLint x, GLint y, GLint z, GLint w);
	static PFNGLVERTEXATTRIBI4IPROC pfnVertexAttribI4i = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4i", pfnVertexAttribI4i, PFNGLVERTEXATTRIBI4IPROC);
	pfnVertexAttribI4i(index, x, y, z, w);
}

void glVertexAttribI1ui(GLuint index, GLuint x)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI1UIPROC)(GLuint index, GLuint x);
	static PFNGLVERTEXATTRIBI1UIPROC pfnVertexAttribI1ui = 0;
	LOAD_ENTRYPOINT("glVertexAttribI1ui", pfnVertexAttribI1ui, PFNGLVERTEXATTRIBI1UIPROC);
	pfnVertexAttribI1ui(index, x);
}

void glVertexAttribI2ui(GLuint index, GLuint x, GLuint y)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI2UIPROC)(GLuint index, GLuint x, GLuint y);
	static PFNGLVERTEXATTRIBI2UIPROC pfnVertexAttribI2ui = 0;
	LOAD_ENTRYPOINT("glVertexAttribI2ui", pfnVertexAttribI2ui, PFNGLVERTEXATTRIBI2UIPROC);
	pfnVertexAttribI2ui(index, x, y);
}

void glVertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI3UIPROC)(GLuint index, GLuint x, GLuint y, GLuint z);
	static PFNGLVERTEXATTRIBI3UIPROC pfnVertexAttribI3ui = 0;
	LOAD_ENTRYPOINT("glVertexAttribI3ui", pfnVertexAttribI3ui, PFNGLVERTEXATTRIBI3UIPROC);
	pfnVertexAttribI3ui(index, x, y, z);
}

void glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4UIPROC)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
	static PFNGLVERTEXATTRIBI4UIPROC pfnVertexAttribI4ui = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4ui", pfnVertexAttribI4ui, PFNGLVERTEXATTRIBI4UIPROC);
	pfnVertexAttribI4ui(index, x, y, z, w);
}

void glVertexAttribI1iv(GLuint index, const GLint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI1IVPROC)(GLuint index, const GLint *v);
	static PFNGLVERTEXATTRIBI1IVPROC pfnVertexAttribI1iv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI1iv", pfnVertexAttribI1iv, PFNGLVERTEXATTRIBI1IVPROC);
	pfnVertexAttribI1iv(index, v);
}

void glVertexAttribI2iv(GLuint index, const GLint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI2IVPROC)(GLuint index, const GLint *v);
	static PFNGLVERTEXATTRIBI2IVPROC pfnVertexAttribI2iv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI2iv", pfnVertexAttribI2iv, PFNGLVERTEXATTRIBI2IVPROC);
	pfnVertexAttribI2iv(index, v);
}

void glVertexAttribI3iv(GLuint index, const GLint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI3IVPROC)(GLuint index, const GLint *v);
	static PFNGLVERTEXATTRIBI3IVPROC pfnVertexAttribI3iv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI3iv", pfnVertexAttribI3iv, PFNGLVERTEXATTRIBI3IVPROC);
	pfnVertexAttribI3iv(index, v);
}

void glVertexAttribI4iv(GLuint index, const GLint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4IVPROC)(GLuint index, const GLint *v);
	static PFNGLVERTEXATTRIBI4IVPROC pfnVertexAttribI4iv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4iv", pfnVertexAttribI4iv, PFNGLVERTEXATTRIBI4IVPROC);
	pfnVertexAttribI4iv(index, v);
}

void glVertexAttribI1uiv(GLuint index, const GLuint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI1UIVPROC)(GLuint index, const GLuint *v);
	static PFNGLVERTEXATTRIBI1UIVPROC pfnVertexAttribI1uiv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI1uiv", pfnVertexAttribI1uiv, PFNGLVERTEXATTRIBI1UIVPROC);
	pfnVertexAttribI1uiv(index, v);
}

void glVertexAttribI2uiv(GLuint index, const GLuint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI2UIVPROC)(GLuint index, const GLuint *v);
	static PFNGLVERTEXATTRIBI2UIVPROC pfnVertexAttribI2uiv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI2uiv", pfnVertexAttribI2uiv, PFNGLVERTEXATTRIBI2UIVPROC);
	pfnVertexAttribI2uiv(index, v);
}

void glVertexAttribI3uiv(GLuint index, const GLuint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI3UIVPROC)(GLuint index, const GLuint *v);
	static PFNGLVERTEXATTRIBI3UIVPROC pfnVertexAttribI3uiv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI3uiv", pfnVertexAttribI3uiv, PFNGLVERTEXATTRIBI3UIVPROC);
	pfnVertexAttribI3uiv(index, v);
}

void glVertexAttribI4uiv(GLuint index, const GLuint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4UIVPROC)(GLuint index, const GLuint *v);
	static PFNGLVERTEXATTRIBI4UIVPROC pfnVertexAttribI4uiv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4uiv", pfnVertexAttribI4uiv, PFNGLVERTEXATTRIBI4UIVPROC);
	pfnVertexAttribI4uiv(index, v);
}

void glVertexAttribI4bv(GLuint index, const GLbyte *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4BVPROC)(GLuint index, const GLbyte *v);
	static PFNGLVERTEXATTRIBI4BVPROC pfnVertexAttribI4bv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4bv", pfnVertexAttribI4bv, PFNGLVERTEXATTRIBI4BVPROC);
	pfnVertexAttribI4bv(index, v);
}

void glVertexAttribI4sv(GLuint index, const GLshort *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4SVPROC)(GLuint index, const GLshort *v);
	static PFNGLVERTEXATTRIBI4SVPROC pfnVertexAttribI4sv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4sv", pfnVertexAttribI4sv, PFNGLVERTEXATTRIBI4SVPROC);
	pfnVertexAttribI4sv(index, v);
}

void glVertexAttribI4ubv(GLuint index, const GLubyte *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4UBVPROC)(GLuint index, const GLubyte *v);
	static PFNGLVERTEXATTRIBI4UBVPROC pfnVertexAttribI4ubv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4ubv", pfnVertexAttribI4ubv, PFNGLVERTEXATTRIBI4UBVPROC);
	pfnVertexAttribI4ubv(index, v);
}

void glVertexAttribI4usv(GLuint index, const GLushort *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4USVPROC)(GLuint index, const GLushort *v);
	static PFNGLVERTEXATTRIBI4USVPROC pfnVertexAttribI4usv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4usv", pfnVertexAttribI4usv, PFNGLVERTEXATTRIBI4USVPROC);
	pfnVertexAttribI4usv(index, v);
}

void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBIPOINTERPROC)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	static PFNGLVERTEXATTRIBIPOINTERPROC pfnVertexAttribIPointer = 0;
	LOAD_ENTRYPOINT("glVertexAttribIPointer", pfnVertexAttribIPointer, PFNGLVERTEXATTRIBIPOINTERPROC);
	pfnVertexAttribIPointer(index, size, type, stride, pointer);
}

void glGetVertexAttribIiv(GLuint index, GLenum pname, GLint *params)
{
	typedef void (APIENTRY * PFNGLGETVERTEXATTRIBIIVPROC)(GLuint index, GLenum pname, GLint *params);
	static PFNGLGETVERTEXATTRIBIIVPROC pfnGetVertexAttribIiv = 0;
	LOAD_ENTRYPOINT("glGetVertexAttribIiv", pfnGetVertexAttribIiv, PFNGLGETVERTEXATTRIBIIVPROC);
	pfnGetVertexAttribIiv(index, pname, params);
}

void glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params)
{
	typedef void (APIENTRY * PFNGLGETVERTEXATTRIBIUIVPROC)(GLuint index, GLenum pname, GLuint *params);
	static PFNGLGETVERTEXATTRIBIUIVPROC pfnGetVertexAttribIuiv = 0;
	LOAD_ENTRYPOINT("glGetVertexAttribIuiv", pfnGetVertexAttribIuiv, PFNGLGETVERTEXATTRIBIUIVPROC);
	pfnGetVertexAttribIuiv(index, pname, params);
}

void glGetUniformuiv(GLuint program, GLint location, GLuint *params)
{
	typedef void (APIENTRY * PFNGLGETUNIFORMUIVPROC)(GLuint program, GLint location, GLuint *params);
	static PFNGLGETUNIFORMUIVPROC pfnGetUniformuiv = 0;
	LOAD_ENTRYPOINT("glGetUniformuiv", pfnGetUniformuiv, PFNGLGETUNIFORMUIVPROC);
	pfnGetUniformuiv(program, location, params);
}

void glBindFragDataLocation(GLuint program, GLuint color, const GLchar *name)
{
	typedef void (APIENTRY * PFNGLBINDFRAGDATALOCATIONPROC)(GLuint program, GLuint color, const GLchar *name);
	static PFNGLBINDFRAGDATALOCATIONPROC pfnBindFragDataLocation = 0;
	LOAD_ENTRYPOINT("glBindFragDataLocation", pfnBindFragDataLocation, PFNGLBINDFRAGDATALOCATIONPROC);
	pfnBindFragDataLocation(program, color, name);
}

GLint glGetFragDataLocation(GLuint program, const GLchar *name)
{
	typedef GLint (APIENTRY * PFNGLGETFRAGDATALOCATIONPROC)(GLuint program, const GLchar *name);
	static PFNGLGETFRAGDATALOCATIONPROC pfnGetFragDataLocation = 0;
	LOAD_ENTRYPOINT("glGetFragDataLocation", pfnGetFragDataLocation, PFNGLGETFRAGDATALOCATIONPROC);
	return pfnGetFragDataLocation(program, name);
}

void glUniform1ui(GLint location, GLuint v0)
{
	typedef void (APIENTRY * PFNGLUNIFORM1UIPROC)(GLint location, GLuint v0);
	static PFNGLUNIFORM1UIPROC pfnUniform1ui = 0;
	LOAD_ENTRYPOINT("glUniform1ui", pfnUniform1ui, PFNGLUNIFORM1UIPROC);
	pfnUniform1ui(location, v0);
}

void glUniform2ui(GLint location, GLuint v0, GLuint v1)
{
	typedef void (APIENTRY * PFNGLUNIFORM2UIPROC)(GLint location, GLuint v0, GLuint v1);
	static PFNGLUNIFORM2UIPROC pfnUniform2ui = 0;
	LOAD_ENTRYPOINT("glUniform2ui", pfnUniform2ui, PFNGLUNIFORM2UIPROC);
	pfnUniform2ui(location, v0, v1);
}

void glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	typedef void (APIENTRY * PFNGLUNIFORM3UIPROC)(GLint location, GLuint v0, GLuint v1, GLuint v2);
	static PFNGLUNIFORM3UIPROC pfnUniform3ui = 0;
	LOAD_ENTRYPOINT("glUniform3ui", pfnUniform3ui, PFNGLUNIFORM3UIPROC);
	pfnUniform3ui(location, v0, v1, v2);
}

void glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	typedef void (APIENTRY * PFNGLUNIFORM4UIPROC)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	static PFNGLUNIFORM4UIPROC pfnUniform4ui = 0;
	LOAD_ENTRYPOINT("glUniform4ui", pfnUniform4ui, PFNGLUNIFORM4UIPROC);
	pfnUniform4ui(location, v0, v1, v2, v3);
}

void glUniform1uiv(GLint location, GLsizei count, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLUNIFORM1UIVPROC)(GLint location, GLsizei count, const GLuint *value);
	static PFNGLUNIFORM1UIVPROC pfnUniform1uiv = 0;
	LOAD_ENTRYPOINT("glUniform1uiv", pfnUniform1uiv, PFNGLUNIFORM1UIVPROC);
	pfnUniform1uiv(location, count, value);
}

void glUniform2uiv(GLint location, GLsizei count, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLUNIFORM2UIVPROC)(GLint location, GLsizei count, const GLuint *value);
	static PFNGLUNIFORM2UIVPROC pfnUniform2uiv = 0;
	LOAD_ENTRYPOINT("glUniform2uiv", pfnUniform2uiv, PFNGLUNIFORM2UIVPROC);
	pfnUniform2uiv(location, count, value);
}

void glUniform3uiv(GLint location, GLsizei count, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLUNIFORM3UIVPROC)(GLint location, GLsizei count, const GLuint *value);
	static PFNGLUNIFORM3UIVPROC pfnUniform3uiv = 0;
	LOAD_ENTRYPOINT("glUniform3uiv", pfnUniform3uiv, PFNGLUNIFORM3UIVPROC);
	pfnUniform3uiv(location, count, value);
}

void glUniform4uiv(GLint location, GLsizei count, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLUNIFORM4UIVPROC)(GLint location, GLsizei count, const GLuint *value);
	static PFNGLUNIFORM4UIVPROC pfnUniform4uiv = 0;
	LOAD_ENTRYPOINT("glUniform4uiv", pfnUniform4uiv, PFNGLUNIFORM4UIVPROC);
	pfnUniform4uiv(location, count, value);
}

void glTexParameterIiv(GLenum target, GLenum pname, const GLint *params)
{
	typedef void (APIENTRY * PFNGLTEXPARAMETERIIVPROC)(GLenum target, GLenum pname, const GLint *params);
	static PFNGLTEXPARAMETERIIVPROC pfnTexParameterIiv = 0;
	LOAD_ENTRYPOINT("glTexParameterIiv", pfnTexParameterIiv, PFNGLTEXPARAMETERIIVPROC);
	pfnTexParameterIiv(target, pname, params);
}

void glTexParameterIuiv(GLenum target, GLenum pname, const GLuint *params)
{
	typedef void (APIENTRY * PFNGLTEXPARAMETERIUIVPROC)(GLenum target, GLenum pname, const GLuint *params);
	static PFNGLTEXPARAMETERIUIVPROC pfnTexParameterIuiv = 0;
	LOAD_ENTRYPOINT("glTexParameterIuiv", pfnTexParameterIuiv, PFNGLTEXPARAMETERIUIVPROC);
	pfnTexParameterIuiv(target, pname, params);
}

void glGetTexParameterIiv(GLenum target, GLenum pname, GLint *params)
{
	typedef void (APIENTRY * PFNGLGETTEXPARAMETERIIVPROC)(GLenum target, GLenum pname, GLint *params);
	static PFNGLGETTEXPARAMETERIIVPROC pfnGetTexParameterIiv = 0;
	LOAD_ENTRYPOINT("glGetTexParameterIiv", pfnGetTexParameterIiv, PFNGLGETTEXPARAMETERIIVPROC);
	pfnGetTexParameterIiv(target, pname, params);
}

void glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint *params)
{
	typedef void (APIENTRY * PFNGLGETTEXPARAMETERIUIVPROC)(GLenum target, GLenum pname, GLuint *params);
	static PFNGLGETTEXPARAMETERIUIVPROC pfnGetTexParameterIuiv = 0;
	LOAD_ENTRYPOINT("glGetTexParameterIuiv", pfnGetTexParameterIuiv, PFNGLGETTEXPARAMETERIUIVPROC);
	pfnGetTexParameterIuiv(target, pname, params);
}

void glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value)
{
	typedef void (APIENTRY * PFNGLCLEARBUFFERIVPROC)(GLenum buffer, GLint drawbuffer, const GLint *value);
	static PFNGLCLEARBUFFERIVPROC pfnClearBufferiv = 0;
	LOAD_ENTRYPOINT("glClearBufferiv", pfnClearBufferiv, PFNGLCLEARBUFFERIVPROC);
	pfnClearBufferiv(buffer, drawbuffer, value);
}

void glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLCLEARBUFFERUIVPROC)(GLenum buffer, GLint drawbuffer, const GLuint *value);
	static PFNGLCLEARBUFFERUIVPROC pfnClearBufferuiv = 0;
	LOAD_ENTRYPOINT("glClearBufferuiv", pfnClearBufferuiv, PFNGLCLEARBUFFERUIVPROC);
	pfnClearBufferuiv(buffer, drawbuffer, value);
}

void glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLCLEARBUFFERFVPROC)(GLenum buffer, GLint drawbuffer, const GLfloat *value);
	static PFNGLCLEARBUFFERFVPROC pfnClearBufferfv = 0;
	LOAD_ENTRYPOINT("glClearBufferfv", pfnClearBufferfv, PFNGLCLEARBUFFERFVPROC);
	pfnClearBufferfv(buffer, drawbuffer, value);
}

void glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
	typedef void (APIENTRY * PFNGLCLEARBUFFERFIPROC)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
	static PFNGLCLEARBUFFERFIPROC pfnClearBufferfi = 0;
	LOAD_ENTRYPOINT("glClearBufferfi", pfnClearBufferfi, PFNGLCLEARBUFFERFIPROC);
	pfnClearBufferfi(buffer, drawbuffer, depth, stencil);
}

const GLubyte *glGetStringi(GLenum name, GLuint index)
{
	typedef const GLubyte *(APIENTRY * PFNGLGETSTRINGIPROC)(GLenum name, GLuint index);
	static PFNGLGETSTRINGIPROC pfnGetStringi = 0;
	LOAD_ENTRYPOINT("glGetStringi", pfnGetStringi, PFNGLGETSTRINGIPROC);
	return pfnGetStringi(name, index);
}

GLboolean glIsRenderbuffer(GLuint renderbuffer)
{
	typedef GLboolean (APIENTRY * PFNGLISRENDERBUFFERPROC)(GLuint renderbuffer);
	static PFNGLISRENDERBUFFERPROC pfnIsRenderbuffer = 0;
	LOAD_ENTRYPOINT("glIsRenderbuffer", pfnIsRenderbuffer, PFNGLISRENDERBUFFERPROC);
	return pfnIsRenderbuffer(renderbuffer);
}

void glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
	typedef void (APIENTRY * PFNGLBINDRENDERBUFFERPROC)(GLenum target, GLuint renderbuffer);
	static PFNGLBINDRENDERBUFFERPROC pfnBindRenderbuffer = 0;
	LOAD_ENTRYPOINT("glBindRenderbuffer", pfnBindRenderbuffer, PFNGLBINDRENDERBUFFERPROC);
	pfnBindRenderbuffer(target, renderbuffer);
}

void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers)
{
	typedef void (APIENTRY * PFNGLDELETERENDERBUFFERSPROC)(GLsizei n, const GLuint *renderbuffers);
	static PFNGLDELETERENDERBUFFERSPROC pfnDeleteRenderbuffers = 0;
	LOAD_ENTRYPOINT("glDeleteRenderbuffers", pfnDeleteRenderbuffers, PFNGLDELETERENDERBUFFERSPROC);
	pfnDeleteRenderbuffers(n, renderbuffers);
}

void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
	typedef void (APIENTRY * PFNGLGENRENDERBUFFERSPROC)(GLsizei n, GLuint *renderbuffers);
	static PFNGLGENRENDERBUFFERSPROC pfnGenRenderbuffers = 0;
	LOAD_ENTRYPOINT("glGenRenderbuffers", pfnGenRenderbuffers, PFNGLGENRENDERBUFFERSPROC);
	pfnGenRenderbuffers(n, renderbuffers);
}

void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
	typedef void (APIENTRY * PFNGLRENDERBUFFERSTORAGEPROC)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLRENDERBUFFERSTORAGEPROC pfnRenderbufferStorage = 0;
	LOAD_ENTRYPOINT("glRenderbufferStorage", pfnRenderbufferStorage, PFNGLRENDERBUFFERSTORAGEPROC);
	pfnRenderbufferStorage(target, internalformat, width, height);
}

void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
	typedef void (APIENTRY * PFNGLGETRENDERBUFFERPARAMETERIVPROC)(GLenum target, GLenum pname, GLint *params);
	static PFNGLGETRENDERBUFFERPARAMETERIVPROC pfnGetRenderbufferParameteriv = 0;
	LOAD_ENTRYPOINT("glGetRenderbufferParameteriv", pfnGetRenderbufferParameteriv, PFNGLGETRENDERBUFFERPARAMETERIVPROC);
	pfnGetRenderbufferParameteriv(target, pname, params);
}

GLboolean glIsFramebuffer(GLuint framebuffer)
{
	typedef GLboolean (APIENTRY * PFNGLISFRAMEBUFFERPROC)(GLuint framebuffer);
	static PFNGLISFRAMEBUFFERPROC pfnIsFramebuffer = 0;
	LOAD_ENTRYPOINT("glIsFramebuffer", pfnIsFramebuffer, PFNGLISFRAMEBUFFERPROC);
	return pfnIsFramebuffer(framebuffer);
}

void glBindFramebuffer(GLenum target, GLuint framebuffer)
{
	typedef void (APIENTRY * PFNGLBINDFRAMEBUFFERPROC)(GLenum target, GLuint framebuffer);
	static PFNGLBINDFRAMEBUFFERPROC pfnBindFramebuffer = 0;
	LOAD_ENTRYPOINT("glBindFramebuffer", pfnBindFramebuffer, PFNGLBINDFRAMEBUFFERPROC);
	pfnBindFramebuffer(target, framebuffer);
}

void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers)
{
	typedef void (APIENTRY * PFNGLDELETEFRAMEBUFFERSPROC)(GLsizei n, const GLuint *framebuffers);
	static PFNGLDELETEFRAMEBUFFERSPROC pfnDeleteFramebuffers = 0;
	LOAD_ENTRYPOINT("glDeleteFramebuffers", pfnDeleteFramebuffers, PFNGLDELETEFRAMEBUFFERSPROC);
	pfnDeleteFramebuffers(n, framebuffers);
}

void glGenFramebuffers(GLsizei n, GLuint *framebuffers)
{
	typedef void (APIENTRY * PFNGLGENFRAMEBUFFERSPROC)(GLsizei n, GLuint *framebuffers);
	static PFNGLGENFRAMEBUFFERSPROC pfnGenFramebuffers = 0;
	LOAD_ENTRYPOINT("glGenFramebuffers", pfnGenFramebuffers, PFNGLGENFRAMEBUFFERSPROC);
	pfnGenFramebuffers(n, framebuffers);
}

GLenum glCheckFramebufferStatus(GLenum target)
{
	typedef GLenum (APIENTRY * PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum target);
	static PFNGLCHECKFRAMEBUFFERSTATUSPROC pfnCheckFramebufferStatus = 0;
	LOAD_ENTRYPOINT("glCheckFramebufferStatus", pfnCheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC);
	return pfnCheckFramebufferStatus(target);
}

void glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURE1DPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	static PFNGLFRAMEBUFFERTEXTURE1DPROC pfnFramebufferTexture1D = 0;
	LOAD_ENTRYPOINT("glFramebufferTexture1D", pfnFramebufferTexture1D, PFNGLFRAMEBUFFERTEXTURE1DPROC);
	pfnFramebufferTexture1D(target, attachment, textarget, texture, level);
}

void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	static PFNGLFRAMEBUFFERTEXTURE2DPROC pfnFramebufferTexture2D = 0;
	LOAD_ENTRYPOINT("glFramebufferTexture2D", pfnFramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC);
	pfnFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
	typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURE3DPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
	static PFNGLFRAMEBUFFERTEXTURE3DPROC pfnFramebufferTexture3D = 0;
	LOAD_ENTRYPOINT("glFramebufferTexture3D", pfnFramebufferTexture3D, PFNGLFRAMEBUFFERTEXTURE3DPROC);
	pfnFramebufferTexture3D(target, attachment, textarget, texture, level, zoffset);
}

void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	typedef void (APIENTRY * PFNGLFRAMEBUFFERRENDERBUFFERPROC)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	static PFNGLFRAMEBUFFERRENDERBUFFERPROC pfnFramebufferRenderbuffer = 0;
	LOAD_ENTRYPOINT("glFramebufferRenderbuffer", pfnFramebufferRenderbuffer, PFNGLFRAMEBUFFERRENDERBUFFERPROC);
	pfnFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
	typedef void (APIENTRY * PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)(GLenum target, GLenum attachment, GLenum pname, GLint *params);
	static PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC pfnGetFramebufferAttachmentParameteriv = 0;
	LOAD_ENTRYPOINT("glGetFramebufferAttachmentParameteriv", pfnGetFramebufferAttachmentParameteriv, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC);
	pfnGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

void glGenerateMipmap(GLenum target)
{
	typedef void (APIENTRY * PFNGLGENERATEMIPMAPPROC)(GLenum target);
	static PFNGLGENERATEMIPMAPPROC pfnGenerateMipmap = 0;
	LOAD_ENTRYPOINT("glGenerateMipmap", pfnGenerateMipmap, PFNGLGENERATEMIPMAPPROC);
	pfnGenerateMipmap(target);
}

void glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
	typedef void (APIENTRY * PFNGLBLITFRAMEBUFFERPROC)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
	static PFNGLBLITFRAMEBUFFERPROC pfnBlitFramebuffer = 0;
	LOAD_ENTRYPOINT("glBlitFramebuffer", pfnBlitFramebuffer, PFNGLBLITFRAMEBUFFERPROC);
	pfnBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	typedef void (APIENTRY * PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC pfnRenderbufferStorageMultisample = 0;
	LOAD_ENTRYPOINT("glRenderbufferStorageMultisample", pfnRenderbufferStorageMultisample, PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC);
	pfnRenderbufferStorageMultisample(target, samples, internalformat, width, height);
}

void glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURELAYERPROC)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
	static PFNGLFRAMEBUFFERTEXTURELAYERPROC pfnFramebufferTextureLayer = 0;
	LOAD_ENTRYPOINT("glFramebufferTextureLayer", pfnFramebufferTextureLayer, PFNGLFRAMEBUFFERTEXTURELAYERPROC);
	pfnFramebufferTextureLayer(target, attachment, texture, level, layer);
}

GLvoid *glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	typedef GLvoid* (APIENTRY * PFNGLMAPBUFFERRANGEPROC)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
	static PFNGLMAPBUFFERRANGEPROC pfnMapBufferRange = 0;
	LOAD_ENTRYPOINT("glMapBufferRange", pfnMapBufferRange, PFNGLMAPBUFFERRANGEPROC);
	return pfnMapBufferRange(target, offset, length, access);
}

void glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
	typedef void (APIENTRY * PFNGLFLUSHMAPPEDBUFFERRANGEPROC)(GLenum target, GLintptr offset, GLsizeiptr length);
	static PFNGLFLUSHMAPPEDBUFFERRANGEPROC pfnFlushMappedBufferRange = 0;
	LOAD_ENTRYPOINT("glFlushMappedBufferRange", pfnFlushMappedBufferRange, PFNGLFLUSHMAPPEDBUFFERRANGEPROC);
	pfnFlushMappedBufferRange(target, offset, length);
}

void glBindVertexArray(GLuint array)
{
	typedef void (APIENTRY * PFNGLBINDVERTEXARRAYPROC)(GLuint array);
	static PFNGLBINDVERTEXARRAYPROC pfnBindVertexArray = 0;
	LOAD_ENTRYPOINT("glBindVertexArray", pfnBindVertexArray, PFNGLBINDVERTEXARRAYPROC);
	pfnBindVertexArray(array);
}

void glDeleteVertexArrays(GLsizei n, const GLuint *arrays)
{
	typedef void (APIENTRY * PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint *arrays);
	static PFNGLDELETEVERTEXARRAYSPROC pfnDeleteVertexArrays = 0;
	LOAD_ENTRYPOINT("glDeleteVertexArrays", pfnDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC);
	pfnDeleteVertexArrays(n, arrays);
}

void glGenVertexArrays(GLsizei n, GLuint *arrays)
{
	typedef void (APIENTRY * PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint *arrays);
	static PFNGLGENVERTEXARRAYSPROC pfnGenVertexArrays = 0;
	LOAD_ENTRYPOINT("glGenVertexArrays", pfnGenVertexArrays, PFNGLGENVERTEXARRAYSPROC);
	pfnGenVertexArrays(n, arrays);
}

GLboolean glIsVertexArray(GLuint array)
{
	typedef GLboolean (APIENTRY * PFNGLISVERTEXARRAYPROC)(GLuint array);
	static PFNGLISVERTEXARRAYPROC pfnIsVertexArray = 0;
	LOAD_ENTRYPOINT("glIsVertexArray", pfnIsVertexArray, PFNGLISVERTEXARRAYPROC);
	return pfnIsVertexArray(array);
}

//
// OpenGL 3.1
//

void glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    typedef void (APIENTRY * PFNGLCOPYBUFFERSUBDATAPROC)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    static PFNGLCOPYBUFFERSUBDATAPROC pfnCopyBufferSubData = 0;
    LOAD_ENTRYPOINT("glCopyBufferSubData", pfnCopyBufferSubData, PFNGLCOPYBUFFERSUBDATAPROC);
    pfnCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
}

void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
    typedef void (APIENTRY * PFNGLDRAWARRAYSINSTANCEDPROC)(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
    static PFNGLDRAWARRAYSINSTANCEDPROC pfnDrawArraysInstanced = 0;
    LOAD_ENTRYPOINT("glDrawArraysInstanced", pfnDrawArraysInstanced, PFNGLDRAWARRAYSINSTANCEDPROC);
    pfnDrawArraysInstanced(mode, first, count, primcount);
}

void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount)
{
    typedef void (APIENTRY * PFNGLDRAWELEMENTSINSTANCEDPROC)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount);
    static PFNGLDRAWELEMENTSINSTANCEDPROC pfnDrawElementsInstanced = 0;
    LOAD_ENTRYPOINT("glDrawElementsInstanced", pfnDrawElementsInstanced, PFNGLDRAWELEMENTSINSTANCEDPROC);
    pfnDrawElementsInstanced(mode, count, type, indices, primcount);
}

void glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)
{
    typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
    static PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC pfnGetActiveUniformBlockName = 0;
    LOAD_ENTRYPOINT("glGetActiveUniformBlockName", pfnGetActiveUniformBlockName, PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC);
    pfnGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

void glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMBLOCKIVPROC)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
    static PFNGLGETACTIVEUNIFORMBLOCKIVPROC pfnGetActiveUniformBlockiv = 0;
    LOAD_ENTRYPOINT("glGetActiveUniformBlockiv", pfnGetActiveUniformBlockiv, PFNGLGETACTIVEUNIFORMBLOCKIVPROC);
    pfnGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
}

void glGetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName)
{
    typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMNAMEPROC)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);
    static PFNGLGETACTIVEUNIFORMNAMEPROC pfnGetActiveUniformName = 0;
    LOAD_ENTRYPOINT("glGetActiveUniformName", pfnGetActiveUniformName, PFNGLGETACTIVEUNIFORMNAMEPROC);
    pfnGetActiveUniformName(program, uniformIndex, bufSize, length, uniformName);
}

void glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMSIVPROC)(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
    static PFNGLGETACTIVEUNIFORMSIVPROC pfnGetActiveUniformsiv = 0;
    LOAD_ENTRYPOINT("glGetActiveUniformsiv", pfnGetActiveUniformsiv, PFNGLGETACTIVEUNIFORMSIVPROC);
    pfnGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
}

GLuint glGetUniformBlockIndex(GLuint program, const GLchar *uniformBlockName)
{
    typedef GLuint (APIENTRY * PFNGLGETUNIFORMBLOCKINDEXPROC)(GLuint program, const GLchar *uniformBlockName);
    static PFNGLGETUNIFORMBLOCKINDEXPROC pfnGetUniformBlockIndex = 0;
    LOAD_ENTRYPOINT("glGetUniformBlockIndex", pfnGetUniformBlockIndex, PFNGLGETUNIFORMBLOCKINDEXPROC);
    return pfnGetUniformBlockIndex(program, uniformBlockName);
}

void glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar **uniformNames, GLuint *uniformIndices)
{
    typedef void (APIENTRY * PFNGLGETUNIFORMINDICESPROC)(GLuint program, GLsizei uniformCount, const GLchar **uniformNames, GLuint *uniformIndices);
    static PFNGLGETUNIFORMINDICESPROC pfnGetUniformIndices = 0;
    LOAD_ENTRYPOINT("glGetUniformIndices", pfnGetUniformIndices, PFNGLGETUNIFORMINDICESPROC);
    pfnGetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
}

void glPrimitiveRestartIndex(GLuint index)
{
    typedef void (APIENTRY * PFNGLPRIMITIVERESTARTINDEXPROC)(GLuint index);
    static PFNGLPRIMITIVERESTARTINDEXPROC pfnPrimitiveRestartIndex = 0;
    LOAD_ENTRYPOINT("glPrimitiveRestartIndex", pfnPrimitiveRestartIndex, PFNGLPRIMITIVERESTARTINDEXPROC);
    pfnPrimitiveRestartIndex(index);
}

void glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer)
{
    typedef void (APIENTRY * PFNGLTEXBUFFERPROC)(GLenum target, GLenum internalformat, GLuint buffer);
    static PFNGLTEXBUFFERPROC pfnTexBuffer = 0;
    LOAD_ENTRYPOINT("glTexBuffer", pfnTexBuffer, PFNGLTEXBUFFERPROC);
    pfnTexBuffer(target, internalformat, buffer);
}

void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    typedef void (APIENTRY * PFNGLUNIFORMBLOCKBINDINGPROC)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
    static PFNGLUNIFORMBLOCKBINDINGPROC pfnUniformBlockBinding = 0;
    LOAD_ENTRYPOINT("glUniformBlockBinding", pfnUniformBlockBinding, PFNGLUNIFORMBLOCKBINDINGPROC);
    pfnUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
}

//
// OpenGL 3.2
//

void glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex)
{
	typedef void (APIENTRY * PFNGLDRAWELEMENTSBASEVERTEXPROC)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex);
	static PFNGLDRAWELEMENTSBASEVERTEXPROC pfnDrawElementsBaseVertex = 0;
	LOAD_ENTRYPOINT("glDrawElementsBaseVertex", pfnDrawElementsBaseVertex, PFNGLDRAWELEMENTSBASEVERTEXPROC);
	pfnDrawElementsBaseVertex(mode, count, type, indices, basevertex);
}

void glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex)
{
	typedef void (APIENTRY * PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex);
	static PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC pfnDrawRangeElementsBaseVertex = 0;
	LOAD_ENTRYPOINT("glDrawRangeElementsBaseVertex", pfnDrawRangeElementsBaseVertex, PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC);
	pfnDrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
}

void glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount, GLint basevertex)
{
	typedef void (APIENTRY * PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount, GLint basevertex);
	static PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC pfnDrawElementsInstancedBaseVertex = 0;
	LOAD_ENTRYPOINT("glDrawElementsInstancedBaseVertex", pfnDrawElementsInstancedBaseVertex, PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC);
	pfnDrawElementsInstancedBaseVertex(mode, count, type, indices, primcount, basevertex);
}

void glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount, const GLint *basevertex)
{
	typedef void (APIENTRY * PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC)(GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount, const GLint *basevertex);
	static PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC pfnMultiDrawElementsBaseVertex = 0;
	LOAD_ENTRYPOINT("glMultiDrawElementsBaseVertex", pfnMultiDrawElementsBaseVertex, PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC);
	pfnMultiDrawElementsBaseVertex(mode, count, type, indices, primcount, basevertex);
}

void glProvokingVertex(GLenum mode)
{
	typedef void (APIENTRY * PFNGLPROVOKINGVERTEXPROC)(GLenum mode);
	static PFNGLPROVOKINGVERTEXPROC pfnProvokingVertex = 0;
	LOAD_ENTRYPOINT("glProvokingVertex", pfnProvokingVertex, PFNGLPROVOKINGVERTEXPROC);
	pfnProvokingVertex(mode);
}

GLsync glFenceSync(GLenum condition, GLbitfield flags)
{
	typedef GLsync (APIENTRY * PFNGLFENCESYNCPROC)(GLenum condition, GLbitfield flags);
	static PFNGLFENCESYNCPROC pfnFenceSync = 0;
	LOAD_ENTRYPOINT("glFenceSync", pfnFenceSync, PFNGLFENCESYNCPROC);
	return pfnFenceSync(condition, flags);
}

GLboolean glIsSync(GLsync sync)
{
	typedef GLboolean (APIENTRY * PFNGLISSYNCPROC)(GLsync sync);
	static PFNGLISSYNCPROC pfnIsSync = 0;
	LOAD_ENTRYPOINT("glIsSync", pfnIsSync, PFNGLISSYNCPROC);
	return pfnIsSync(sync);
}

void glDeleteSync(GLsync sync)
{
	typedef void (APIENTRY * PFNGLDELETESYNCPROC)(GLsync sync);
	static PFNGLDELETESYNCPROC pfnDeleteSync = 0;
	LOAD_ENTRYPOINT("glDeleteSync", pfnDeleteSync, PFNGLDELETESYNCPROC);
	pfnDeleteSync(sync);
}

GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	typedef GLenum (APIENTRY * PFNGLCLIENTWAITSYNCPROC)(GLsync sync, GLbitfield flags, GLuint64 timeout);
	static PFNGLCLIENTWAITSYNCPROC pfnClientWaitSync = 0;
	LOAD_ENTRYPOINT("glClientWaitSync", pfnClientWaitSync, PFNGLCLIENTWAITSYNCPROC);
	return pfnClientWaitSync(sync, flags, timeout);
}

void glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	typedef void (APIENTRY * PFNGLWAITSYNCPROC)(GLsync sync, GLbitfield flags, GLuint64 timeout);
	static PFNGLWAITSYNCPROC pfnWaitSync = 0;
	LOAD_ENTRYPOINT("glWaitSync", pfnWaitSync, PFNGLWAITSYNCPROC);
	pfnWaitSync(sync, flags, timeout);
}

void glGetInteger64v(GLenum pname, GLint64 *params)
{
	typedef void (APIENTRY * PFNGLGETINTEGER64VPROC)(GLenum pname, GLint64 *params);
	static PFNGLGETINTEGER64VPROC pfnGetInteger64v = 0;
	LOAD_ENTRYPOINT("glGetInteger64v", pfnGetInteger64v, PFNGLGETINTEGER64VPROC);
	pfnGetInteger64v(pname, params);
}

void glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)
{
	typedef void (APIENTRY * PFNGLGETSYNCIVPROC)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
	static PFNGLGETSYNCIVPROC pfnGetSynciv = 0;
	LOAD_ENTRYPOINT("glGetSynciv", pfnGetSynciv, PFNGLGETSYNCIVPROC);
	pfnGetSynciv(sync, pname, bufSize, length, values);
}

void glTexImage2DMultisample(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
	typedef void (APIENTRY * PFNGLTEXIMAGE2DMULTISAMPLEPROC)(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
	static PFNGLTEXIMAGE2DMULTISAMPLEPROC pfnTexImage2DMultisample = 0;
	LOAD_ENTRYPOINT("glTexImage2DMultisample", pfnTexImage2DMultisample, PFNGLTEXIMAGE2DMULTISAMPLEPROC);
	pfnTexImage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

void glTexImage3DMultisample(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
	typedef void (APIENTRY * PFNGLTEXIMAGE3DMULTISAMPLEPROC)(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
	static PFNGLTEXIMAGE3DMULTISAMPLEPROC pfnTexImage3DMultisample = 0;
	LOAD_ENTRYPOINT("glTexImage3DMultisample", pfnTexImage3DMultisample, PFNGLTEXIMAGE3DMULTISAMPLEPROC);
	pfnTexImage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

void glGetMultisamplefv(GLenum pname, GLuint index, GLfloat *val)
{
	typedef void (APIENTRY * PFNGLGETMULTISAMPLEFVPROC)(GLenum pname, GLuint index, GLfloat *val);
	static PFNGLGETMULTISAMPLEFVPROC pfnGetMultisamplefv = 0;
	LOAD_ENTRYPOINT("glGetMultisamplefv", pfnGetMultisamplefv, PFNGLGETMULTISAMPLEFVPROC);
	pfnGetMultisamplefv(pname, index, val);
}

void glSampleMaski(GLuint index, GLbitfield mask)
{
	typedef void (APIENTRY * PFNGLSAMPLEMASKIPROC)(GLuint index, GLbitfield mask);
	static PFNGLSAMPLEMASKIPROC pfnSampleMaski = 0;
	LOAD_ENTRYPOINT("glSampleMaski", pfnSampleMaski, PFNGLSAMPLEMASKIPROC);
	pfnSampleMaski(index, mask);
}

void glGetInteger64i_v(GLenum target, GLuint index, GLint64 *data)
{
	typedef void (APIENTRY * PFNGLGETINTEGER64I_VPROC)(GLenum target, GLuint index, GLint64 *data);
	static PFNGLGETINTEGER64I_VPROC pfnGetInteger64i_v = 0;
	LOAD_ENTRYPOINT("glGetInteger64i_v", pfnGetInteger64i_v, PFNGLGETINTEGER64I_VPROC);
	pfnGetInteger64i_v(target, index, data);
}

void glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64 *params)
{
	typedef void (APIENTRY * PFNGLGETBUFFERPARAMETERI64VPROC)(GLenum target, GLenum pname, GLint64 *params);
	static PFNGLGETBUFFERPARAMETERI64VPROC pfnGetBufferParameteri64v = 0;
	LOAD_ENTRYPOINT("glGetBufferParameteri64v", pfnGetBufferParameteri64v, PFNGLGETBUFFERPARAMETERI64VPROC);
	pfnGetBufferParameteri64v(target, pname, params);
}

void glProgramParameteri(GLuint program, GLenum pname, GLint value)
{
	typedef void (APIENTRY * PFNGLPROGRAMPARAMETERIPROC)(GLuint program, GLenum pname, GLint value);
}

void glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTUREPROC)(GLenum target, GLenum attachment, GLuint texture, GLint level);
	static PFNGLFRAMEBUFFERTEXTUREPROC pfnFramebufferTexture = 0;
	LOAD_ENTRYPOINT("glFramebufferTexture", pfnFramebufferTexture, PFNGLFRAMEBUFFERTEXTUREPROC);
	pfnFramebufferTexture(target, attachment, texture, level);
}

void glFramebufferTextureFace(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face)
{
	typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTUREFACEPROC)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);
	static PFNGLFRAMEBUFFERTEXTUREFACEPROC pfnFramebufferTextureFace = 0;
	LOAD_ENTRYPOINT("glFramebufferTextureFace", pfnFramebufferTextureFace, PFNGLFRAMEBUFFERTEXTUREFACEPROC);
	pfnFramebufferTextureFace(target, attachment, texture, level, face);
}