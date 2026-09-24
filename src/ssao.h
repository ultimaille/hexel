#pragma once

#include "core/core.h"

struct SSAO : RenderLayer {
    const std::string name = "ssao";
    GLuint quad_vao = 0;
    GLuint quad_vbo = 0;

    SSAO() {
        visible = true;
    }

    ~SSAO() override {
        destroy();
    }

    void generate_gui(std::string) override {}
    bool resync_with_data() override { return true; }

    void init() {
        God::shaders.add(std::string(SHADERS_DIR), name);
        initialize_quad();
    }

    void render() override {
        GLuint program = God::shaders[name];
        RenderTarget& target = God::context.render_target;
        if (!target.valid())
            return;


        // allocate a temporary copy target for this render pass only
        RenderTarget copy;
        copy.init(target.width, target.height);
        copy_target_to_source(target, copy);

        target.bind();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        glUseProgram(program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, copy.color);
        glUniform1i(glGetUniformLocation(program, "source_color"), 0);

        draw_quad();
    }

private:
    void copy_target_to_source(const RenderTarget& source, const RenderTarget& destination) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, source.framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination.framebuffer);
        glBlitFramebuffer(
            0, 0, source.width, source.height,
            0, 0, destination.width, destination.height,
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
            GL_NEAREST
        );
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
    }

    void draw_quad() {
        glBindVertexArray(quad_vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
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
    }
};

