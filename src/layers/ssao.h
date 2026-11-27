#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <unordered_map>
#include <core/all.h>

struct SSAO : Layer {
    // a framebuffer with a single texture: GL_R8 (occlusion) or GL_R32F (view z), optionally with mip levels
    struct Target {
        GLuint framebuffer = 0;
        GLuint texture = 0;
        int width = 0;
        int height = 0;
        int levels = 0;

        Target() = default;
        Target(const Target&) = delete;
        Target& operator=(const Target&) = delete;
        ~Target() { destroy(); }

        // (re)allocate only when the size or the number of levels changes
        void ensure(int w, int h, GLenum internal_format, int nb_levels = 1) {
            um_assert(w > 0 && h > 0 && nb_levels > 0);
            um_assert(internal_format == GL_R8 || internal_format == GL_R32F);
            if (texture && w == width && h == height && nb_levels == levels)
                return;
            destroy();
            width = w;
            height = h;
            levels = nb_levels;

            const GLenum type = internal_format == GL_R8 ? GL_UNSIGNED_BYTE : GL_FLOAT;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            for (int l = 0; l < levels; ++l)
                glTexImage2D(GL_TEXTURE_2D, l, internal_format, level_width(l), level_height(l), 0, GL_RED, type, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels - 1);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, levels > 1 ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glGenFramebuffers(1, &framebuffer);
            bind(0);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                Log::error("SSAO::Target framebuffer is incomplete");
        }

        // bind for drawing into mip level `level`, and set the viewport accordingly
        void bind(int level = 0) const {
            um_assert(framebuffer && level < levels);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, level);
            glViewport(0, 0, level_width(level), level_height(level));
        }

        int level_width (int l) const { return std::max(1, width  >> l); }
        int level_height(int l) const { return std::max(1, height >> l); }

        void destroy() {
            if (texture)     glDeleteTextures(1, &texture);
            if (framebuffer) glDeleteFramebuffers(1, &framebuffer);
            texture = framebuffer = 0;
            width = height = levels = 0;
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
        void set(const char *name, int x, int y)     { glUniform2i(location(name), x, y); }
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

    Pass z_pass, downsample_pass, ao_pass, blur_pass, composite_pass;

    GLuint empty_vao = 0; // the core profile requires a bound VAO, even to draw without attributes

    // intermediate buffers, kept across frames and only reallocated on resize
    Target z;    // view-space z pyramid: distant samples read coarser levels
    Target ao;
    Target blur;
    static constexpr int max_levels = 7;

    // parameters
    float radius = 0;      // world units; 0 means automatic: a fraction of the scene's size, set once there is geometry
    float falloff = .8f;   // fraction of the radius over which occluders fade out
    float power = 1.5f;    // contrast of the occlusion
    int   slice_count = 3; // directions per pixel (each marched both ways)
    int   step_count = 12; // samples per direction and side

    SSAO() {
        visible = false;
    }

    ~SSAO() override {
        destroy();
    }

    void init() {
        z_pass.init("ssao_z");
        downsample_pass.init("ssao_downsample");
        ao_pass.init("ssao");
        blur_pass.init("ssao_blur");
        composite_pass.init("ssao_composite");
        glGenVertexArrays(1, &empty_vao);
    }

    void render() override {
        if (!visible)
            return;

        RenderTarget& target = God::context.render_target.get_binded_target();
        if (!target.valid())
            return;

        // the occlusion is computed at half resolution, and upsampled by the composite
        const int w = (target.width  + 1) / 2;
        const int h = (target.height + 1) / 2;
        const int levels = std::min(max_levels, 1 + int(std::log2(std::min(w, h))));
        z.ensure(w, h, GL_R32F, levels);
        ao.ensure(w, h, GL_R8);
        blur.ensure(w, h, GL_R8);

        // orthographic projection, row major: ndc = P * view, with w = 1
        std::array<float, 16> P;
        std::copy_n(God::camera.projection_ptr(), 16, P.begin());
        if (radius <= 0) { // automatic
            const double diagonal = scene_diagonal();
            if (diagonal > 0)
                radius = float(.75 * diagonal);
        }
        const float r = radius > 0 ? radius : 2.f / P[5]; // no geometry yet: the view height (2 / P11), not stored
        const float pixel_size = 2.f / P[5] / float(target.height); // in world units, at full resolution

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_BLEND);

        { // view z: target depth -> z level 0, at half resolution
            z.bind(0);
            z_pass.use();
            z_pass.texture("source_depth", 0, target.depth);
            z_pass.set("depth_to_z", 1.f / P[10], -P[11] / P[10]); // z_view = (ndc_z - P23) / P22
            draw_fullscreen();
        }

        { // z pyramid: level l-1 -> level l
            downsample_pass.use();
            downsample_pass.texture("source_z", 0, z.texture);
            downsample_pass.set("depth_range", .75f * r);
            for (int l = 1; l < levels; ++l) {
                // only level l-1 is readable while level l is written: no feedback loop
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, l - 1);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  l - 1);
                z.bind(l);
                draw_fullscreen();
            }
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  levels - 1);
        }

        { // ambient occlusion: z pyramid -> ao
            ao.bind();
            ao_pass.use();
            ao_pass.texture("view_z", 0, z.texture);
            ao_pass.set("uv_to_view", 2.f / P[0], 2.f / P[5], (-1.f - P[3]) / P[0], (-1.f - P[7]) / P[5]); // x_view = (2 uv.x - 1 - P03) / P00, same for y
            ao_pass.set("texel_uv", 2.f / float(target.width), 2.f / float(target.height)); // a level 0 texel covers 2x2 pixels
            ao_pass.set("radius_px", r / (2.f * pixel_size)); // in level 0 texels
            const float falloff_range = std::max(1e-6f, falloff * r);
            ao_pass.set("falloff_mul", -1.f / falloff_range);
            ao_pass.set("falloff_add", r / falloff_range); // weight 1 up to radius - range, 0 at radius
            ao_pass.set("power", power);
            ao_pass.set("slice_count", slice_count);
            ao_pass.set("step_count", step_count);
            ao_pass.set("max_level", levels - 1);
            draw_fullscreen();
        }

        { // separable plane-aware blur, on the half resolution grid: ao -> blur -> ao
            blur_pass.use();
            blur_pass.texture("view_z", 1, z.texture);
            blur_pass.set("tolerance", 4.f * pixel_size);

            blur.bind();
            blur_pass.texture("source_ao", 0, ao.texture);
            blur_pass.set("direction", 1, 0);
            draw_fullscreen();

            ao.bind();
            blur_pass.texture("source_ao", 0, blur.texture);
            blur_pass.set("direction", 0, 1);
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
            composite_pass.texture("view_z", 1, z.texture);
            composite_pass.texture("source_depth", 2, target.depth);
            composite_pass.set("depth_to_z", 1.f / P[10], -P[11] / P[10]);
            composite_pass.set("tolerance", 4.f * pixel_size); // depth difference that halves an ao texel's weight
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

    // diagonal of the bounding box of the scene (visible or not), 0 when there is no geometry
    static double scene_diagonal() {
        BBox3 box;
        for (auto &[name, mm] : God::xcf) // all the meshes of a multimesh share its point set
            box.add(UM::Inspect<PointSet>(mm.pointset).bbox());
        return box.empty() ? 0. : box.size().norm();
    }

    void draw_fullscreen() const {
        glBindVertexArray(empty_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void destroy() override {
        z.destroy();
        ao.destroy();
        blur.destroy();

        if (empty_vao != 0) {
            glDeleteVertexArrays(1, &empty_vao);
            empty_vao = 0;
        }
    }
};

