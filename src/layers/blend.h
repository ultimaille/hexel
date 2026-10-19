#pragma once

#include <array>
#include <core/all.h>

struct RenderTargetComposition {
    GLuint quad_vao = 0;
    GLuint quad_vbo = 0;
    int w, h;

    RenderTargetComposition() {

    }

    ~RenderTargetComposition() {
        destroy();
    }

    void init() {
        God::shaders.create_if_needed(std::string(SHADERS_DIR), "blend");
        composite.init(God::context.render_target.width, God::context.render_target.height);
        initialize_quad();
    }

    void blend(GLuint a, GLuint b, Group::BlendOp blend_op) {
        // Use blend shader
        GLuint blend_program = God::shaders["blend"];
        glUseProgram(blend_program);
        
        glUniform1i(glGetUniformLocation(blend_program, "mix"), static_cast<int>(blend_op));

        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, a);
        glUniform1i(glGetUniformLocation(blend_program, "tex_a"), 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, b);
        glUniform1i(glGetUniformLocation(blend_program, "tex_b"), 1);
        
        draw_quad();
    }

    void render() {
        
        const int w = God::context.render_target.width;
        const int h = God::context.render_target.height;

        std::vector<int> group_to_target(God::layers.groups.size(), -1);
        std::vector<RenderTarget> targets;
        targets.reserve(God::layers.groups.size()); // pre-allocation
        targets.emplace_back();
        targets[0].init(w, h);
        targets[0].clear();

        for (int i = 0; i < God::layers.groups.size(); ++i) {
            auto &group = God::layers.groups[i];

            // Only create a new render target if needed (when a merge option selected)
            // only create if not first group
            if (i > 0 && group.blend_op != Group::BlendOp::NONE) {
                targets.emplace_back();
                targets[targets.size() - 1].init(w, h);
                targets[targets.size() - 1].clear(); // clear & bind
            }

            group_to_target[i] = targets.size() - 1;

            // Normal rendering of layers of current group
            for (int l = 0; l < God::layers.size(); ++l) {
                Layer &layer = God::layers[l];
                if (layer.group == group.id)
                    layer.render();
            }
        }

        if (targets.size() == 0)
            return;

        composite.resize(w, h);
        composite.clear();
        // Merge all color attachments into main render target, keeping its depth
        glBindFramebuffer(GL_FRAMEBUFFER, composite.framebuffer);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);  // Don't write to depth buffer
        glDepthFunc(GL_ALWAYS); // Pass all depth tests (use existing depth)

        blend(composite.color, targets[0].color, Group::BlendOp::NONE);
        // Render all framebuffer colors as layers
        if (targets.size() > 1) {
            for (int i = 0; i < God::layers.groups.size(); ++i) {
                auto &group = God::layers.groups[i];
                if (group.blend_op != Group::BlendOp::NONE)
                    blend(composite.color, targets[group_to_target[i]].color, group.blend_op);
            }
        }

        glDepthMask(GL_TRUE);   // Re-enable depth writes
        glDisable(GL_DEPTH_TEST);
    }

    void display() {
        composite.copy_to(God::context.render_target);
    }

    void read_framebuffer(std::vector<unsigned char>& data, int x, int y, int w, int h, int attachment, int group) {

        if (group < 0 || group > God::layers.groups.size() - 1)
            return;

        
    }

private:

    RenderTarget composite;

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

