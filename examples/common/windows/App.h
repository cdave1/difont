#ifndef _DIFONT_EXAMPLES_APP_H_
#define _DIFONT_EXAMPLES_APP_H_

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <memory>
#include <string>
#include <vector>



#include "HelloWorld.h"

#include <difont/opengl/OpenGLInterface.h>
#include "WGL_ARB_create_context.h"
#include "WGL_ARB_multisample.h"
#include "WGL_EXT_swap_control.h"

#include <string>
static HelloWorld *helloWorld;

class App
{
public:
	App();
	App(const wchar_t *pszWindowName);
	virtual ~App();

	void preferAnisotropicTextureFiltering();
	void preferAnisotropicTextureFiltering(int degreeOfAnisotrophy);
	void preferAntiAliasing();
	void preferAntiAliasing(int samples);
	void preferFullScreen();
	void preferWindowed();
	int run();


	GLuint loadFragmentShader(const char *pszFilename);
	GLuint loadVertexShader(const char *pszFilename);
	GLuint loadShaderProgram(const char *pszVertexShaderFilename, const char *pszFragmentShaderFilename);
	GLuint loadShader(const char *pszFilename, GLenum shaderType) const;
	inline std::string programInfoLog() const;
	std::string programInfoLog(GLuint program) const;


	inline std::string shaderInfoLog() const;
	std::string shaderInfoLog(GLuint shader) const;
protected:
	void displayFullScreen();
	void displayWindowed();
	void enableVerticalSync(bool enableVerticalSync);
	void quit(int status = 0);

	inline bool isFullScreen() const;

	void toggleFullScreen();
	void toggleVerticalSync();

	bool verticalSyncEnabled() const;

	inline int viewportHeight() const;
	inline int viewportWidth() const;

protected:
	void init(int argc, char *argv[]);
	void render();
	void shutdown();
	void update(float elapsedTimeSec);

private:
	static float elapsedTimeInSeconds();
	static bool getEncoderClsid(const wchar_t *pszFormat, CLSID *pClsid);
	static LRESULT CALLBACK windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool create();
	void destroy();
	void initOpenGL();
	void initApplication(const wchar_t *pszWindowName);
	int mainLoop();
	LRESULT windowProcImpl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static const int BEST_ANTI_ALIASING_SAMPLES = -1;
	static const int BEST_DEGREE_OF_ANISOTROPY = -1;
	static const int MAX_SAMPLE_COUNT = 50;

	static INT64     m_freq;
	static INT64     m_lastTime;
	static int       m_sampleCount;
	static float     m_timeScale;
	static float     m_actualElapsedTimeSec;
	static float     m_frameTimes[MAX_SAMPLE_COUNT];
	static bool      m_initialized;

	const wchar_t *m_pszWindowName;
	HWND        m_hWnd;
	HINSTANCE   m_hInstance;
	HDC         m_hDC;
	HGLRC       m_hRC;
	int         m_windowWidth;
	int         m_windowHeight;
	int         m_framesPerSecond;
	int         m_status;
	int         m_antiAliasingSamples;
	int         m_maxDegreeOfAnisotropy;
	int         m_degreeOfAnisotropy;
	DWORD       m_savedExStyle;
	DWORD       m_savedStyle;
	DWORD       m_fileId;
	bool        m_isFullScreen;
	bool        m_hasFocus;
	bool        m_startFullScreen;
	RECT        m_rcSaved;
	WNDCLASSEX  m_wcl;
	CLSID       m_clsidEncoderPng;
	std::string m_shaderInfoLog;
	std::string m_programInfoLog;
};
inline std::string App::programInfoLog() const
{
	return m_programInfoLog;
}

inline std::string App::shaderInfoLog() const
{
	return m_shaderInfoLog;
}
#endif