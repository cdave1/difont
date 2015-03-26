# difont

A font rendering library for OpenGL, OpenGLES, and DirectX.  The library is a fork of ftgles, and should be considered as completely incompatible with that library.

## Goals for this project

Support font rendering on the following platforms:
1.	Windows (OpenGL 3 and DirectX)
2.  OSX (OpenGL 3)
3.  (TODO) iOS (OpenGLES 2)
4.  (TODO) Android (OpenGLES 2)

Client programs can retrieve the font data in a number of different formats, depending on the goals of the project (efficiency, ease-of-use, etc):
1.  Mesh data (Collections of vertex and texture coordinate arrays).
2.  (TODO) OpenGL or DirectX labels (e.g. VertexArrayBuffer).
3.  (TODO) A difont structure that can be used to render fonts quickly without knowing the font contents beforehand.

Additionally, a client program can call simple utility functions to render vertexes to a shader.