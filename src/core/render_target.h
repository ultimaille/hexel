#pragma once

#include <glad/gl.h>

#include <ultimaille/all.h>
using namespace UM;

struct RenderTarget {
    GLuint framebuffer = 0;
    GLuint color = 0;
    GLuint depth = 0;
    GLuint layer = 0;
    GLuint primitive = 0;

    int width  = 0;
    int height = 0;

    RenderTarget() = default;
    RenderTarget(const RenderTarget&) = delete;
    RenderTarget& operator=(const RenderTarget&) = delete;

    ~RenderTarget();
    bool valid() const;

    void init(int w, int h);

    void resize(int w, int h);

    void bind() const;

    static void bind_default(int w, int h);

    void clear(float r = 0, float g = 0, float b = 0, float a = 1) const;

    void destroy();

    void allocate(int w, int h);

    
    void read_framebuffer(std::vector<unsigned char>& data, vec4 rect, int attachment);
};

