#pragma once

#include <array>
#include "core/core.h"

struct SSAO : RenderLayer {
    const std::string name = "ssao";
    GLuint quad_vao = 0;
    GLuint quad_vbo = 0;
    GLuint random_texture = 0;

    SSAO() {
        visible = true;
    }

    ~SSAO() override {
        destroy();
    }


    void generate_gui(std::string) override {
    }

    bool handle(Event) override { return true; }

    void init() {
        God::shaders.add(std::string(SHADERS_DIR), name);
        initialize_quad();
        initialize_random_texture();
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

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, random_texture);
        glUniform1i(glGetUniformLocation(program, "random_texture"), 2);

        glUniformMatrix4fv(glGetUniformLocation(program, "inverse_projection"), 1, GL_TRUE, God::camera.inverse_projection());

        glUniform1f(glGetUniformLocation(program, "max_radius"), 0.5f);
        glUniform1f(glGetUniformLocation(program, "step_mul"), 1.2f);       

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

    void initialize_random_texture() {
        constexpr int width = 32;
        constexpr int height = 32;

        std::array<float, width * height> values;

        // Deterministic pseudo-random values in [0, 1).
        // A fixed pattern makes debugging reproducible.
        uint32_t state = 0x12345678u;

        for (float& value : values) {
            state = 1664525u * state + 1013904223u;
            value = float(state & 0x00ffffffu) /
                float(0x01000000u);
        }

        glGenTextures(1, &random_texture);
        glBindTexture(GL_TEXTURE_2D, random_texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, values.data());
    }

    void draw_quad() {
        glBindVertexArray(quad_vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    void destroy() {
        if (random_texture != 0) {
            glDeleteTextures(1, &random_texture);
            random_texture = 0;
        }

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

