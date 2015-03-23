// App.cpp stolen from http://www.dhpoware.com/demos/gl3HelloWorld.html to get something going quickly.

#pragma comment (lib, "gdiplus.lib")

#include <windows.h>
#include <gdiplus.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <string>
#include <sstream>
#include "stdafx.h"

#if defined(DEBUG) || defined(_DEBUG)
#include <crtdbg.h>
#endif
#include "App.h"
#include "CommonMath.h"

#if !defined(GL_TEXTURE_MAX_ANISOTROPY_EXT)
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif

#if !defined(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT)
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif


INT64 App::m_freq;
INT64 App::m_lastTime;
int   App::m_sampleCount;
float App::m_timeScale;
float App::m_actualElapsedTimeSec;
float App::m_frameTimes[MAX_SAMPLE_COUNT];
bool  App::m_initialized;


App::App()
{
	initApplication(L"");
}

App::App(const wchar_t *pszWindowName)
{
	initApplication(pszWindowName);
}

App::~App()
{
}

void App::preferAnisotropicTextureFiltering()
{
	m_degreeOfAnisotropy = BEST_DEGREE_OF_ANISOTROPY;
}

void App::preferAnisotropicTextureFiltering(int degreeOfAnisotrophy)
{
	m_degreeOfAnisotropy = degreeOfAnisotrophy;
}

void App::preferAntiAliasing()
{
	m_antiAliasingSamples = BEST_ANTI_ALIASING_SAMPLES;
}

void App::preferAntiAliasing(int samples)
{
	m_antiAliasingSamples = samples;
}

void App::preferFullScreen()
{
	m_startFullScreen = true;
}

void App::preferWindowed()
{
	m_startFullScreen = false;
}

int App::run()
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	int status = 0;
	ULONG_PTR gdiplusToken = 0;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;

	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, 0);
	getEncoderClsid(L"image/png", &m_clsidEncoderPng);

	try
	{
		if (create())
		{
			try
			{
				initOpenGL();
				init(__argc, __argv);
				status = mainLoop();
				shutdown();
			}
			catch (...)
			{
				shutdown();
				throw;
			}

			destroy();
		}
	}
	catch (const std::exception &e)
	{
		destroy();
		status = EXIT_FAILURE;

		MessageBox(0, L"Exception", L"GL3Application Unhandled Exception", MB_ICONERROR);
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);

	return status;
}

//-----------------------------------------------------------------------------
// Protected methods.
//-----------------------------------------------------------------------------

void App::displayFullScreen()
{
	if (!m_isFullScreen)
		toggleFullScreen();
}

void App::displayWindowed()
{
	if (m_isFullScreen)
		toggleFullScreen();
}
GLuint App::loadFragmentShader(const char *pszFilename)
{
	return loadShader(pszFilename, GL_FRAGMENT_SHADER);
}

GLuint App::loadVertexShader(const char *pszFilename)
{
	return loadShader(pszFilename, GL_VERTEX_SHADER);
}

GLuint App::loadShader(const char *pszFilename, GLenum shaderType) const
{
	std::string source;
	std::ifstream file(pszFilename, std::ios::binary);

	if (file.is_open())
	{
		file.seekg(0, std::ios::end);

		unsigned int fileSize = static_cast<unsigned int>(file.tellg());

		source.resize(fileSize);
		file.seekg(0, std::ios::beg);
		file.read(&source[0], fileSize);
	}

	GLuint shader = glCreateShader(shaderType);
	const GLchar *pszSource = reinterpret_cast<const GLchar *>(source.c_str());

	glShaderSource(shader, 1, &pszSource, 0);

	return shader;
}

GLuint App::loadShaderProgram(const char *pszVertexShaderFilename, const char *pszFragmentShaderFilename)
{
	GLint compiled = 0;
	GLint linked = 0;
	GLuint vertexShader = 0;
	GLuint fragmentShader = 0;
	GLuint shaderProgram = 0;

	vertexShader = loadVertexShader(pszVertexShaderFilename);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		m_shaderInfoLog = shaderInfoLog(vertexShader);
		glDeleteShader(vertexShader);

		std::ostringstream output;

		output << "Error compiling GLSL vertex shader: '";
		output << pszVertexShaderFilename;
		output << "'" << std::endl << std::endl;
		output << "Shader info log:" << std::endl;
		output << m_shaderInfoLog;

		m_shaderInfoLog = output.str();
	}
	else
	{
		fragmentShader = loadFragmentShader(pszFragmentShaderFilename);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);

		if (!compiled)
		{
			m_shaderInfoLog = shaderInfoLog(fragmentShader);
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			std::ostringstream output;

			output << "Error compiling GLSL fragment shader: '";
			output << pszFragmentShaderFilename;
			output << "'" << std::endl << std::endl;
			output << "Shader info log:" << std::endl;
			output << m_shaderInfoLog;

			m_shaderInfoLog = output.str();
		}
		else
		{
			shaderProgram = glCreateProgram();
			glAttachShader(shaderProgram, vertexShader);
			glAttachShader(shaderProgram, fragmentShader);
			glLinkProgram(shaderProgram);
			glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);

			// Mark the two attached shaders for deletion. These two shaders
			// aren't deleted right now because both are already attached to a
			// shader program. When the shader program is deleted these two
			// shaders will be automatically detached and deleted.

			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			if (!linked)
			{
				m_programInfoLog = programInfoLog(shaderProgram);
				glDeleteProgram(shaderProgram);

				std::ostringstream output;

				output << "Error linking GLSL shaders into a shader program." << std::endl;
				output << "GLSL vertex shader: '" << pszVertexShaderFilename;
				output << "'" << std::endl;
				output << "GLSL fragment shader: '" << pszFragmentShaderFilename;
				output << "'" << std::endl << std::endl;
				output << "Program info log:" << std::endl;
				output << m_programInfoLog;

				m_programInfoLog = output.str();
			}
		}
	}

	return shaderProgram;
}


std::string App::programInfoLog(GLuint program) const
{
	GLsizei infoLogSize = 0;
	std::string infoLog;

	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogSize);
	infoLog.resize(infoLogSize);
	glGetProgramInfoLog(program, infoLogSize, &infoLogSize, &infoLog[0]);

	return infoLog;
}

std::string App::shaderInfoLog(GLuint shader) const
{
	GLsizei infoLogSize = 0;
	std::string infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogSize);
	infoLog.resize(infoLogSize);
	glGetShaderInfoLog(shader, infoLogSize, &infoLogSize, &infoLog[0]);

	return infoLog;
}


void App::quit(int status)
{
	m_status = status;
	PostMessage(m_hWnd, WM_CLOSE, 0, 0);
}

void App::toggleFullScreen()
{
	m_isFullScreen = !m_isFullScreen;

	if (m_isFullScreen)
	{
		// Moving to full screen mode.

		m_savedExStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
		m_savedStyle = GetWindowLong(m_hWnd, GWL_STYLE);
		GetWindowRect(m_hWnd, &m_rcSaved);

		SetWindowLong(m_hWnd, GWL_EXSTYLE, 0);
		SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
		SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

		m_windowWidth = GetSystemMetrics(SM_CXSCREEN);
		m_windowHeight = GetSystemMetrics(SM_CYSCREEN);

		SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, m_windowWidth, m_windowHeight, SWP_SHOWWINDOW);
	}
	else
	{
		// Moving back to windowed mode.

		SetWindowLong(m_hWnd, GWL_EXSTYLE, m_savedExStyle);
		SetWindowLong(m_hWnd, GWL_STYLE, m_savedStyle);
		SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

		m_windowWidth = m_rcSaved.right - m_rcSaved.left;
		m_windowHeight = m_rcSaved.bottom - m_rcSaved.top;

		SetWindowPos(m_hWnd, HWND_NOTOPMOST, m_rcSaved.left, m_rcSaved.top, m_windowWidth, m_windowHeight, SWP_SHOWWINDOW);
	}
}

bool App::verticalSyncEnabled() const
{
	return (wglGetSwapIntervalEXT() == 0) ? false : true;
}

//-----------------------------------------------------------------------------
// Private methods.
//-----------------------------------------------------------------------------

bool App::create()
{
	if (!RegisterClassEx(&m_wcl))
		return false;

	// Create a window that is centered on the desktop. It's exactly 1/4 the
	// size of the desktop. Don't allow it to be resized.

	DWORD wndExStyle = WS_EX_OVERLAPPEDWINDOW;
	DWORD wndStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	m_hWnd = CreateWindowEx(wndExStyle, m_wcl.lpszClassName, m_pszWindowName, wndStyle, 0, 0, 0, 0, 0, 0, m_wcl.hInstance, this);

	if (!m_hWnd)
	{
		UnregisterClass(m_wcl.lpszClassName, m_hInstance);
		return false;
	}
	else
	{
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		int halfScreenWidth = screenWidth / 2;
		int halfScreenHeight = screenHeight / 2;
		int left = (screenWidth - halfScreenWidth) / 2;
		int top = (screenHeight - halfScreenHeight) / 2;
		RECT rc = { 0 };

		SetRect(&rc, left, top, left + halfScreenWidth, top + halfScreenHeight);
		AdjustWindowRectEx(&rc, wndStyle, FALSE, wndExStyle);
		MoveWindow(m_hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

		GetClientRect(m_hWnd, &rc);
		m_windowWidth = rc.right - rc.left;
		m_windowHeight = rc.bottom - rc.top;

		return true;
	}
}

void App::destroy()
{
	if (m_hDC)
	{
		if (m_hRC)
		{
			wglMakeCurrent(m_hDC, 0);
			wglDeleteContext(m_hRC);
			m_hRC = 0;
		}

		ReleaseDC(m_hWnd, m_hDC);
		m_hDC = 0;
	}

	UnregisterClass(m_wcl.lpszClassName, m_hInstance);
}

void App::initOpenGL()
{
	if (!(m_hDC = GetDC(m_hWnd)))
		throw std::runtime_error("GetDC() failed.");

	int pf = 0;
	PIXELFORMATDESCRIPTOR pfd = { 0 };
	OSVERSIONINFO osvi = { 0 };

	// Create and set a pixel format for the window.

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	//if (!GetVersionEx(&osvi))
	//	throw std::runtime_error("GetVersionEx() failed.");

		pf = ChoosePixelFormat(m_hDC, &pfd);

	if (!SetPixelFormat(m_hDC, pf, &pfd))
		throw std::runtime_error("SetPixelFormat() failed.");

	// When running under Windows Vista or later support desktop composition.
	// This doesn't really apply when running in full screen mode.

	//if (osvi.dwMajorVersion > 6 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 0))
		pfd.dwFlags |= PFD_SUPPORT_COMPOSITION;

	// Verify that this OpenGL implementation supports the required extensions.

	if (!gl::ExtensionSupported("WGL_ARB_create_context"))
		throw std::runtime_error("Required extension WGL_ARB_create_context is not supported.");

	if (gl::ExtensionSupported("GL_EXT_texture_filter_anisotropic"))
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_maxDegreeOfAnisotropy);
	else
		m_maxDegreeOfAnisotropy = 1;

	// Create an OpenGL 3.x rendering context.
	// Order of preference is: OpenGL 3.2, OpenGL 3.1, and OpenGL 3.0 forward compatible.

	int attribListGL32[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 2,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	int attribListGL31[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 1,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		0
	};

	int attribListGL30[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 0,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		0
	};

	//if (!(m_hRC = wglCreateContextAttribsARB(m_hDC, 0, attribListGL32)))
	//{
	//	if (!(m_hRC = wglCreateContextAttribsARB(m_hDC, 0, attribListGL31)))
	//	{
			if (!(m_hRC = wglCreateContextAttribsARB(m_hDC, 0, attribListGL30)))
				throw std::runtime_error("wglCreateContextAttribsARB() failed for OpenGL 3 context.");
	//	}
	//}

	if (!wglMakeCurrent(m_hDC, m_hRC))
		throw std::runtime_error("wglMakeCurrent() failed for OpenGL 3 context.");

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	std::string path = "C:\\Users\\david\\Projects\\difont\\examples\\common\\Fonts\\Diavlo_BLACK_II_37.otf";
	helloWorld = new HelloWorld(1800, 1200, 1.0f);
	helloWorld->SetupFonts(path.c_str());
}

void App::initApplication(const wchar_t *pszWindowName)
{
	m_pszWindowName = pszWindowName;
	m_hWnd = 0;
	m_hInstance = GetModuleHandle(0);
	m_hDC = 0;
	m_hRC = 0;
	m_windowWidth = 0;
	m_windowHeight = 0;
	m_framesPerSecond = 0;
	m_status = 0;
	m_antiAliasingSamples = 0;
	m_maxDegreeOfAnisotropy = 1;
	m_degreeOfAnisotropy = 1;
	m_savedExStyle = 0;
	m_savedStyle = 0;
	m_fileId = 0;
	m_isFullScreen = false;
	m_hasFocus = false;
	m_startFullScreen = false;

	memset(&m_rcSaved, 0, sizeof(m_rcSaved));
	memset(&m_wcl, 0, sizeof(m_wcl));

	m_wcl.cbSize = sizeof(m_wcl);
	m_wcl.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	m_wcl.lpfnWndProc = windowProc;
	m_wcl.cbClsExtra = 0;
	m_wcl.cbWndExtra = 0;
	m_wcl.hInstance = m_hInstance;
	m_wcl.hIcon = LoadIcon(0, IDI_APPLICATION);
	m_wcl.hCursor = LoadCursor(0, IDC_ARROW);
	m_wcl.hbrBackground = 0;
	m_wcl.lpszMenuName = 0;
	m_wcl.lpszClassName = L"AppWindowClass";
	m_wcl.hIconSm = 0;

	memset(&m_clsidEncoderPng, 0, sizeof(m_clsidEncoderPng));


}

int App::mainLoop()
{
	if (m_startFullScreen)
		displayFullScreen();

	MSG msg;

	memset(&msg, 0, sizeof(msg));
	ShowWindow(m_hWnd, SW_SHOWDEFAULT);
	UpdateWindow(m_hWnd);

	while (true)
	{
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;

		if (m_hasFocus)
		{
			update(elapsedTimeInSeconds());
			render();
			SwapBuffers(m_hDC);
		}
		else
		{
			WaitMessage();
		}
	}

	return static_cast<int>(msg.wParam);
}

LRESULT App::windowProcImpl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_ACTIVATE:
		switch (wParam)
		{
		default:
			break;

		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			m_hasFocus = true;
			break;

		case WA_INACTIVE:
			if (m_isFullScreen)
				ShowWindow(hWnd, SW_MINIMIZE);
			m_hasFocus = false;
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(m_status);
		return 0;

	case WM_SIZE:
		m_windowWidth = static_cast<int>(LOWORD(lParam));
		m_windowHeight = static_cast<int>(HIWORD(lParam));
		break;

	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Static methods.
//-----------------------------------------------------------------------------

float App::elapsedTimeInSeconds()
{
	// Returns the elapsed time (in seconds) since the last time this method
	// was called. This elaborate setup is to guard against large spikes in
	// the time returned by QueryPerformanceCounter().

	INT64 time = 0;
	float elapsedTimeSec = 0.0f;

	if (!m_initialized)
	{
		m_initialized = true;
		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&m_freq));
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_lastTime));
		m_timeScale = 1.0f / m_freq;
	}

	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time));
	elapsedTimeSec = (time - m_lastTime) * m_timeScale;
	m_lastTime = time;

	if (fabsf(elapsedTimeSec - m_actualElapsedTimeSec) < 1.0f)
	{
		memmove(&m_frameTimes[1], m_frameTimes, sizeof(m_frameTimes) - sizeof(m_frameTimes[0]));
		m_frameTimes[0] = elapsedTimeSec;

		if (m_sampleCount < MAX_SAMPLE_COUNT)
			++m_sampleCount;
	}

	m_actualElapsedTimeSec = 0.0f;

	for (int i = 0; i < m_sampleCount; ++i)
		m_actualElapsedTimeSec += m_frameTimes[i];

	if (m_sampleCount > 0)
		m_actualElapsedTimeSec /= m_sampleCount;

	return m_actualElapsedTimeSec;
}
bool App::getEncoderClsid(const wchar_t *pszFormat, CLSID *pClsid)
{
	UINT num = 0;
	UINT size = 0;

	Gdiplus::GetImageEncodersSize(&num, &size);

	if (!size)
		return false;

	Gdiplus::ImageCodecInfo *pImageCodecInfo = static_cast<Gdiplus::ImageCodecInfo *>(malloc(size));

	if (!pImageCodecInfo)
		return false;

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT i = 0; i < num; ++i)
	{
		if (wcscmp(pImageCodecInfo[i].MimeType, pszFormat) == 0)
		{
			memcpy(pClsid, &pImageCodecInfo[i].Clsid, sizeof(CLSID));
			free(pImageCodecInfo);
			return true;
		}
	}

	free(pImageCodecInfo);
	return false;
}

LRESULT CALLBACK App::windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	App *pApplication = 0;

	if (msg == WM_NCCREATE)
	{
		pApplication = reinterpret_cast<App *>((reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams);
		SetWindowLongPtr(hWnd, GWL_USERDATA, reinterpret_cast<LONG_PTR>(pApplication));
	}
	else
	{
		pApplication = reinterpret_cast<App *>(GetWindowLongPtr(hWnd, GWL_USERDATA));
	}

	if (!pApplication)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	pApplication->m_hWnd = hWnd;
	return pApplication->windowProcImpl(hWnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Virtual methods.
//-----------------------------------------------------------------------------
static  GLuint shaderProgram;
void App::init(int argc, char *argv[])
{
	if (!(shaderProgram = loadShaderProgram("..\\..\\..\\common\\Shaders\\quad.vert", "..\\..\\..\\common\\Shaders\\quad.frag")))
	{
		if (programInfoLog().length() > 0)
			throw std::runtime_error(programInfoLog());

		if (shaderInfoLog().length() > 0)
			throw std::runtime_error(shaderInfoLog());
	}
}

void App::render()
{
	glClearColor(0.3f, 0.5f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (helloWorld) {
        helloWorld->Render(shaderProgram);
	}
}

void App::shutdown()
{
}

void App::update(float elapsedTimeSec)
{
	if (helloWorld) {
		helloWorld->Update(shaderProgram);
	}
}


//-----------------------------------------------------------------------------
// WinMain - application entry point.
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	App app;
	app.preferAnisotropicTextureFiltering();
	app.preferAntiAliasing();
	return app.run();
}