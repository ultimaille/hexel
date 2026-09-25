#pragma once

#include <glad/gl.h>

#include <ultimaille/all.h>

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

    ~RenderTarget() {
        destroy();
    }

    bool valid() const {
        return framebuffer != 0 && color != 0 && depth != 0 && layer != 0 && primitive != 0 && width > 0 && height > 0;
    }

    void init(int w, int h) {
        um_assert(w > 0);
        um_assert(h > 0);
        um_assert(framebuffer == 0);
        um_assert(color == 0);
        um_assert(depth == 0);

        glGenFramebuffers(1, &framebuffer);
        glGenTextures(1, &color);
        glGenTextures(1, &layer);
        glGenTextures(1, &primitive);
        glGenTextures(1, &depth);
        allocate(w, h);
    }

    void resize(int w, int h) {
        um_assert(w > 0);
        um_assert(h > 0);

        if (framebuffer == 0) {
            init(w, h);
            return;
        }

        allocate(w, h);
    }

    void bind() const {
        um_assert(framebuffer != 0 && color != 0 && depth != 0 && width > 0 && height > 0);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, width, height);
    }

    static void bind_default(int w, int h) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, w, h);
    }

    // void clear(float r = 0, float g = 0, float b = 0, float a = 1) const {
    //     bind();

    //     glEnable(GL_DEPTH_TEST);
    //     glDepthFunc(GL_LESS);
    //     glClearColor(r, g, b, a);
    //     glClearDepth(1.);
    //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // }

    void clear(float r = 0, float g = 0, float b = 0, float a = 1) const {
        bind();
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        // Clear attachment 0 (color)
        float color_clear[4] = {r, g, b, a};
        glClearBufferfv(GL_COLOR, 0, color_clear);

        // Clear attachment 1 & 2 (layer and primitive IDs) to zero/background
        float zero_clear[4] = {0.0f, 0.0f, 1.0f, 1.0f};
        glClearBufferfv(GL_COLOR, 1, zero_clear);
        glClearBufferfv(GL_COLOR, 2, zero_clear);

        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void destroy() {
        if (depth) {
            glDeleteTextures(1, &depth);
            depth = 0;
        }
        if (color) {
            glDeleteTextures(1, &color);
            color = 0;
        }
        if (framebuffer) {
            glDeleteFramebuffers(1, &framebuffer);
            framebuffer = 0;
        }
        width = height = 0;
    }

    void allocate(int w, int h) {
        um_assert(w > 0);
        um_assert(h > 0);
        um_assert(framebuffer != 0);
        um_assert(color != 0);
        um_assert(layer != 0);
        um_assert(primitive != 0);
        um_assert(depth != 0);

        width = w;
        height = h;

        // /!\ Explicit-binding policy: do not restore previous GL state.
        // The caller owns and restores the bindings it needs.

        // color buffer
        glBindTexture(GL_TEXTURE_2D, color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // depth buffer
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

        // layer id buffer
        glBindTexture(GL_TEXTURE_2D, layer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // primitive id buffer
        glBindTexture(GL_TEXTURE_2D, primitive);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // framebuffer attachments
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, layer, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, primitive, 0);

        constexpr GLenum draw_buffers[] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2
        };

        glDrawBuffers(3, draw_buffers);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
            Log::error("RenderTarget framebuffer is incomplete");
    }

    // void read_framebuffer(std::vector<unsigned char> &data, vec4 rect, int attachment) {

    //     const auto [x, y, w, h] = std::tuple{rect[0], rect[1], rect[2], rect[3]};
        
    //     // Bind fbo
    //     glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    //     // Get convenient color attachment according to pick primitive id
    //     glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);

    //     data.resize(w * h * 4);

    //     // Read pixels
    //     glReadPixels(x, h - y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    //     // Check for errors
    //     GLenum err = glGetError();
    //     if (err != GL_NO_ERROR)
    //         Log::error("Picking glReadPixels error: " + err);

    //     // Unbind fbo
    //     glReadBuffer(GL_COLOR_ATTACHMENT0);
    //     glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    // }

    void read_framebuffer(std::vector<unsigned char> &data, vec4 rect, int attachment) {
        int x = static_cast<int>(rect[0]);
        int y = static_cast<int>(rect[1]);
        int w = static_cast<int>(rect[2]);
        int h = static_cast<int>(rect[3]);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);

        data.resize(w * h * 4);

        // Flip Y coordinate from Window (Top-Left origin) to OpenGL (Bottom-Left origin)
        int gl_y = height - y - h;

        // Standard alignment setup to avoid stride issues
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
            Log::error("Picking glReadPixels error: " + std::to_string(err));

        // glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }
};

