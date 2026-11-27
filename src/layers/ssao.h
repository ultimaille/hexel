#pragma once

#include <array>
#include <core/all.h>

struct SSAO : Layer {
    struct Target {
        GLuint framebuffer = 0;
        GLuint texture = 0;
        int width = 0;
        int height = 0;

        Target() = default;
        Target(const Target&) = delete;
        Target& operator=(const Target&) = delete;
        ~Target() { destroy(); }

        void ensure(int w, int h) {
            um_assert(w > 0 && h > 0);
            if (texture && w == width && h == height)
                return;
            destroy();
            width = w;
            height = h;

            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glGenFramebuffers(1, &framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                Log::error("SSAO::Target framebuffer is incomplete");
        }

        // bind for drawing, and set the viewport accordingly
        void bind() const {
            um_assert(framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glViewport(0, 0, width, height);
        }

        void destroy() {
            if (texture)     glDeleteTextures(1, &texture);
            if (framebuffer) glDeleteFramebuffers(1, &framebuffer);
            texture = framebuffer = 0;
            width = height = 0;
        }
    };

    const std::string ao_name = "ssao";
    const std::string blur_name = "ssao_blur";
    const std::string composite_name = "ssao_composite";

    GLuint quad_vao = 0;
    GLuint quad_vbo = 0;
    GLuint random_texture = 0;

    // intermediate buffers, kept across frames and only reallocated on resize
    Target ao;
    Target blur;

    SSAO() {
        visible = false;
    }

    ~SSAO() override {
        destroy();
    }

    void init() {
        God::shaders.create_if_needed(std::string(SHADERS_DIR), ao_name);
        God::shaders.create_if_needed(std::string(SHADERS_DIR), blur_name);
        God::shaders.create_if_needed(std::string(SHADERS_DIR), composite_name);
        initialize_quad();
        initialize_random_texture();

        // sampler units never change: set them once
        GLuint p = God::shaders[ao_name];
        glUseProgram(p);
        glUniform1i(glGetUniformLocation(p, "source_depth"), 0);
        glUniform1i(glGetUniformLocation(p, "random_texture"), 1);

        p = God::shaders[blur_name];
        glUseProgram(p);
        glUniform1i(glGetUniformLocation(p, "source_ao"), 0);
        glUniform1i(glGetUniformLocation(p, "source_depth"), 1);

        p = God::shaders[composite_name];
        glUseProgram(p);
        glUniform1i(glGetUniformLocation(p, "source_ao"), 0);
    }

    void render() override {
        if (!visible)
            return;

        RenderTarget& target = God::context.render_target.get_binded_target();
        if (!target.valid())
            return;

        ao.ensure(target.width, target.height);
        blur.ensure(target.width, target.height);

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_BLEND);

        { // ambient occlusion: target depth -> ao
            GLuint program = God::shaders[ao_name];
            ao.bind();
            glUseProgram(program);
            bind_texture(0, target.depth);
            bind_texture(1, random_texture);
            glUniformMatrix4fv(glGetUniformLocation(program, "inverse_projection"), 1, GL_TRUE, God::camera.inverse_projection_ptr());
            glUniform1f(glGetUniformLocation(program, "max_radius"), 0.5f);
            glUniform1f(glGetUniformLocation(program, "step_mul"), 1.2f);
            draw_quad();
        }

        { // separable depth-aware blur: ao -> blur -> ao
            GLuint program = God::shaders[blur_name];
            glUseProgram(program);
            glUniform1i(glGetUniformLocation(program, "blur_radius"), 10);
            bind_texture(1, target.depth); // the blur rejects samples across depth discontinuities of the scene

            blur.bind();
            bind_texture(0, ao.texture);
            glUniform2f(glGetUniformLocation(program, "texel_step"), 1.0f / float(target.width), 0.0f);
            draw_quad();

            ao.bind();
            bind_texture(0, blur.texture);
            glUniform2f(glGetUniformLocation(program, "texel_step"), 0.0f, 1.0f / float(target.height));
            draw_quad();
        }

        { // composite: multiply the target's color by the ao, in place
            target.bind();
            constexpr GLenum attachment0 = GL_COLOR_ATTACHMENT0;
            glDrawBuffers(1, &attachment0); // leave the layer/primitive id attachments untouched (picking re-renders this layer)
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE); // rgb *= ao, alpha unchanged

            glUseProgram(God::shaders[composite_name]);
            bind_texture(0, ao.texture);
            draw_quad();

            target.draw_ids(target.ids_enabled); // restore the target's draw buffers
        }

        // TODO this does not match the philosophy of "each guy must setup its own environment"
        glBlendFunc(GL_ONE, GL_ZERO); // GL default, nobody else sets it
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
    }

private:

    static void bind_texture(int unit, GLuint texture) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texture);
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
        constexpr int width  = 32;
        constexpr int height = 32;

        std::array<float, width * height> values;
        uint32_t state = 0x12345678u;
        for (float& value : values) {
            state = 1664525u * state + 1013904223u;
            value = float(state & 0x00ffffffu) / float(0x01000000u);
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

    void destroy() override {
        ao.destroy();
        blur.destroy();

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
