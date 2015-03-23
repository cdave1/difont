#version 330

precision highp float;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

in vec3 Position;

void main() {
    mat4 mvpMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix;
    vec4 pos = vec4(Position, 1.0);
    gl_Position = mvpMatrix * pos;
}
