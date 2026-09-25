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

        // temporary copy of the current render target
        RenderTarget copy;
        copy.init(target.width, target.height);
        copy_target_to_source(target, copy);

        target.bind();

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_BLEND);

        glUseProgram(program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, copy.color);
        glUniform1i(glGetUniformLocation(program, "source_color"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, copy.depth);
        glUniform1i(glGetUniformLocation(program, "source_depth"), 1);

        glUniformMatrix4fv(glGetUniformLocation(program, "inverse_projection"), 1, GL_TRUE, God::camera.inverse_projection());
        glUniform2f(glGetUniformLocation(program, "texel_size"), 1/float(target.width), 1/float(target.height));

        glUniform1f(glGetUniformLocation(program, "horizon_radius_pixels"), 40.0f);
        glUniform1f(glGetUniformLocation(program, "horizon_bias"), 0.02f);
        glUniform1i(glGetUniformLocation(program, "horizon_steps"), 16);
        glUniform1f(glGetUniformLocation(program, "ao_strength"), 2.0f);

        draw_quad();

        // TODO this does not match the philosophy of "each guy must setup its own environment"
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }

private:
    void copy_target_to_source(const RenderTarget& source, const RenderTarget& destination) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, source.framebuffer);
        um_assert(glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination.framebuffer);
        um_assert(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

        glBlitFramebuffer(
            0, 0, source.width, source.height,
            0, 0, destination.width, destination.height,
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
            GL_NEAREST
        );

        GLenum error = glGetError();
        um_assert(error == GL_NO_ERROR);
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

