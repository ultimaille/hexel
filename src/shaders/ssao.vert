#version 330 core

// Shared by all SSAO passes: a quad covering the whole viewport, generated from gl_VertexID (no vertex buffer).
// Draw with glDrawArrays(GL_TRIANGLE_STRIP, 0, 4) and an empty VAO. TexCoord spans [0,1] over the viewport.

out vec2 TexCoord;

void main() {
    vec2 p = vec2(gl_VertexID & 1, gl_VertexID >> 1); // (0,0) (1,0) (0,1) (1,1)
    TexCoord = p;
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
