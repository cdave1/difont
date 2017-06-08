# difont

A font rendering library for OpenGL 3, OpenGLES 3, and DirectX.  The library is a hard fork of ftgles.

## Goals for this project

Support font rendering on the following platforms:
 * Windows (OpenGL 3 and DirectX)
 * OSX (OpenGL 3)
 * Android (OpenGLES 3) (No example program!)
 * (TODO) iOS (OpenGLES 3)
 * (TODO) Android (OpenGLES 2)

Client programs can retrieve the font data in a number of different formats, depending on the goals of the project (efficiency, ease-of-use, etc):
 * Mesh data (Collections of vertex and texture coordinate arrays).
 * (TODO) OpenGL or DirectX labels (e.g. VertexArrayBuffer).
 * (TODO) A difont structure that can be used to render fonts quickly without knowing the font contents beforehand.

Additionally, a client program can call simple utility functions to render vertexes to a shader.
