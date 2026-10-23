#pragma once

#include <core/core.h>
#include <core/layers.h>
#include <core/shaders.h>
#include <core/render_target.h>

struct RenderTargetComposition {

    RenderTargetComposition() {

    }

    ~RenderTargetComposition() {
        destroy();
    }

    void init(int w, int h);

    void resize(int w, int h) {
        composite.resize(w, h);
    }

    void blend(GLuint a, GLuint b, Group::BlendOp blend_op);

    void begin();
    void end();

    void bind(int group_id);

    void clear(float r, float g, float b, float a) {
        composite.clear(r, g, b, a);
    }

    bool valid() {
        return composite.valid();
    }

    std::vector<int> buffer_indexes() {
        std::vector<int> result;
        for (auto &[buffer_idx, _] : targets) {
            result.push_back(buffer_idx);
        }
        return result;
    }

    void destroy();

    RenderTarget& get_binded_target() {
        return targets[cur_target];
    }

    GLuint quad_vao = 0;
    GLuint quad_vbo = 0;
    int width, height;
    RenderTarget composite;
    std::map<int, RenderTarget> targets;

private:

    int cur_target = 0;

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

};

