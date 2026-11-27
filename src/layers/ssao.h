#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <unordered_map>
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

    // a shader program run over the whole viewport: ssao.vert linked with <name>.frag.
    struct Pass {
        GLuint program = 0;

        void init(const std::string &fragment_name) {
            God::shaders.create_if_needed(std::string(SHADERS_DIR), fragment_name, "ssao");
            program = God::shaders[fragment_name];
            locations.clear();
        }

        void use() const { glUseProgram(program); }

        // uniform locations are looked up once and cached.
        GLint location(const char *name) {
            auto it = locations.find(name);
            if (it == locations.end())
                it = locations.emplace(name, glGetUniformLocation(program, name)).first;
            return it->second;
        }

        void set(const char *name, int v)            { glUniform1i(location(name), v);    }
        void set(const char *name, float v)          { glUniform1f(location(name), v);    }
        void set(const char *name, float x, float y) { glUniform2f(location(name), x, y); }
        void set(const char *name, float x, float y, float z, float w) { glUniform4f(location(name), x, y, z, w); }

        // bind texture to unit and point the sampler uniform name at it
        void texture(const char *name, int unit, GLuint texture) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, texture);
            set(name, unit);
        }

    private:
        std::unordered_map<std::string, GLint> locations;
    };

    Pass ao_pass, blur_pass, composite_pass;

    GLuint empty_vao = 0; // the core profile requires a bound VAO, even to draw without attributes
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
        ao_pass.init("ssao");
        blur_pass.init("ssao_blur");
        composite_pass.init("ssao_composite");
        glGenVertexArrays(1, &empty_vao);
        initialize_random_texture();
    }

    void render() override {
        if (!visible)
            return;

        RenderTarget& target = God::context.render_target.get_binded_target();
        if (!target.valid())
            return;

        ao.ensure(target.width, target.height);
        blur.ensure(target.width, target.height);

        // orthographic projection, row major: ndc = P * view, with w = 1
        std::array<float, 16> P;
        std::copy_n(God::camera.projection_ptr(), 16, P.begin());

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_BLEND);

        { // ambient occlusion: target depth -> ao
            ao.bind();
            ao_pass.use();
            ao_pass.texture("source_depth", 0, target.depth);
            ao_pass.texture("random_texture", 1, random_texture);
            ao_pass.set("depth_to_z", 1.f / P[10], -P[11] / P[10]);                                        // z_view = (ndc_z - P23) / P22
            ao_pass.set("uv_to_view", 2.f / P[0], 2.f / P[5], (-1.f - P[3]) / P[0], (-1.f - P[7]) / P[5]); // x_view = (2 uv.x - 1 - P03) / P00, same for y
            ao_pass.set("max_radius", 0.5f);
            ao_pass.set("step_mul", 1.2f);
            draw_fullscreen();
        }

        { // separable depth-aware blur: ao -> blur -> ao
            blur_pass.use();
            blur_pass.set("blur_radius", 10);
            blur_pass.texture("source_depth", 1, target.depth); // the blur rejects samples across depth discontinuities of the scene

            blur.bind();
            blur_pass.texture("source_ao", 0, ao.texture);
            blur_pass.set("texel_step", 1.0f / float(target.width), 0.0f);
            draw_fullscreen();

            ao.bind();
            blur_pass.texture("source_ao", 0, blur.texture);
            blur_pass.set("texel_step", 0.0f, 1.0f / float(target.height));
            draw_fullscreen();
        }

        { // composite: multiply the target's color by the ao, in place
            target.bind();
            constexpr GLenum attachment0 = GL_COLOR_ATTACHMENT0;
            glDrawBuffers(1, &attachment0); // leave the layer/primitive id attachments untouched (picking re-renders this layer)
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE); // rgb *= ao, alpha unchanged

            composite_pass.use();
            composite_pass.texture("source_ao", 0, ao.texture);
            draw_fullscreen();

            target.draw_ids(target.ids_enabled); // restore the target's draw buffers
        }

        // TODO this does not match the philosophy of "each guy must setup its own environment"
        glBlendFunc(GL_ONE, GL_ZERO); // GL default, nobody else sets it
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
    }

private:

    void draw_fullscreen() const {
        glBindVertexArray(empty_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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

    void destroy() override {
        ao.destroy();
        blur.destroy();

        if (random_texture != 0) {
            glDeleteTextures(1, &random_texture);
            random_texture = 0;
        }

        if (empty_vao != 0) {
            glDeleteVertexArrays(1, &empty_vao);
            empty_vao = 0;
        }
    }
};

