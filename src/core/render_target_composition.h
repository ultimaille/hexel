#pragma once

#include <array>
#include <core/all.h>

struct RenderTargetComposition {
    GLuint quad_vao = 0;
    GLuint quad_vbo = 0;
    int width, height;

    RenderTargetComposition() {

    }

    ~RenderTargetComposition() {
        destroy();
    }

    void init(int w, int h) {
        God::shaders.create_if_needed(std::string(SHADERS_DIR), "blend");
        composite.init(w, h);
        initialize_quad();
        width = w;
        height = h;
    }

    void resize(int w, int h) {
        composite.resize(w, h);
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

    void begin() {
        for (int i = 0; i < God::layers.groups.size(); ++i) {
            auto &group = God::layers.groups[i];

            // Only create a new render target if needed (when a merge option selected)
            // only create if not first group
            if (i == 0 || i > 0 && group.blend_op != Group::BlendOp::NONE) {
                targets[group.id].init(width, height);
            }
        }
    }

    void bind(int group_id) {
        if (targets.contains(group_id))
            targets[group_id].clear(); // clear & bind framebuffer
    }

    void clear(float r, float g, float b, float a) {
        composite.clear(r, g, b, a);
    }

    void end() {
        if (targets.size() == 0)
            return;

        composite.resize(width, height); // Need ?
        composite.clear();
        // Merge all color attachments into main render target, keeping its depth
        glBindFramebuffer(GL_FRAMEBUFFER, composite.framebuffer);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);  // Don't write to depth buffer
        glDepthFunc(GL_ALWAYS); // Pass all depth tests (use existing depth)

        // Render all framebuffer colors as layers        
        for (int i = 0; i < God::layers.groups.size(); ++i) {
            auto &group = God::layers.groups[i];
            if (i == 0 || group.blend_op != Group::BlendOp::NONE) {
                if (targets.contains(group.id))
                    blend(composite.color, targets[group.id].color, group.blend_op);
            }
        }

        glDepthMask(GL_TRUE);   // Re-enable depth writes
        glDisable(GL_DEPTH_TEST);

        targets.clear();
    }

    void read_framebuffer(std::vector<unsigned char>& data, int x, int y, int w, int h, int attachment, int group_id) {
        um_assert(targets.contains(group_id));
        targets.at(group_id).read_framebuffer(data, x, y, w, h, attachment);
    }

    void read_depth(std::vector<float>& data, int x, int y, int w, int h, int group_id) {
        um_assert(targets.contains(group_id));
        targets.at(group_id).read_depth(data, x, y, w, h);
    }

    std::vector<int> buffer_groups() {
        std::vector<int> result;
        for (auto &[group_id, _] : targets) {
            result.push_back(group_id);
        }
        return result;
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


    RenderTarget composite;

private:

    std::map<int, RenderTarget> targets;

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

