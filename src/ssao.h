#pragma once

#include "core/core.h"

struct SSAO : RenderLayer {
    GLuint program  = 0;
    GLuint quad_vao = 0;
    GLuint quad_vbo = 0;
    GLuint source_framebuffer = 0;
    GLuint source_color       = 0;
    GLuint source_depth       = 0;

    int source_width  = 0;
    int source_height = 0;

    SSAO() {
        visible = true;
    }

    ~SSAO() override {
        destroy();
    }

    void generate_gui(std::string) override {}
    bool resync_with_data() override { return true; }

    void init() {
        God::shaders.add(std::string(SHADERS_DIR), "fullscreen_lennon");
        program = God::shaders["fullscreen_lennon"];
        initialize_quad();
        resize(
            God::context.render_target.width,
            God::context.render_target.height
        );
    }

    void resize(int width, int height) { // TODO this one is not called on resize event. resync_with_data?
        if (width <= 0 || height <= 0)
            return;

        if (!source_framebuffer) {
            glGenFramebuffers(1, &source_framebuffer);
            glGenTextures(1, &source_color);
            glGenTextures(1, &source_depth);
        }

        source_width  = width;
        source_height = height;

        // source color texture
        glBindTexture(GL_TEXTURE_2D, source_color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, source_width, source_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // source depth texture
        glBindTexture(GL_TEXTURE_2D, source_depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, source_width, source_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

        // source framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, source_framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, source_color, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, source_depth, 0);

        um_assert( glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE );
    }

    void render() override {
        RenderTarget& target = God::context.render_target;
        copy_target_to_source(target);

        // the source is now safe to sample

        target.bind();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        glUseProgram(program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, source_color);

        glUniform1i(glGetUniformLocation(program, "source_color"), 0);

        draw_quad();
    }

private:
    void copy_target_to_source(const RenderTarget& target) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, target.framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, source_framebuffer);
        glBlitFramebuffer(0, 0, target.width, target.height, 0, 0, source_width, source_height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void initialize_quad() {
        constexpr float vertices[] = {
            // position    // texcoord
            -1, -1,        0, 0,
             1, -1,        1, 0,
             1,  1,        1, 1,
            -1,  1,        0, 1
        };

        glGenVertexArrays(1, &quad_vao);
        glGenBuffers(1, &quad_vbo);
        glBindVertexArray(quad_vao);

        glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void draw_quad() {
        glBindVertexArray(quad_vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
    }

    void destroy() {
        if (quad_vbo != 0) {
            glDeleteBuffers(1, &quad_vbo);
            quad_vbo = 0;
        }

        if (quad_vao != 0) {
            glDeleteVertexArrays(1, &quad_vao);
            quad_vao = 0;
        }

        if (source_depth != 0) {
            glDeleteTextures(1, &source_depth);
            source_depth = 0;
        }

        if (source_color != 0) {
            glDeleteTextures(1, &source_color);
            source_color = 0;
        }

        if (source_framebuffer != 0) {
            glDeleteFramebuffers(1, &source_framebuffer);
            source_framebuffer = 0;
        }
    }
};

