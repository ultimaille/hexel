#pragma once

#include <array>
#include <core/all.h>

struct Blend {
    GLuint quad_vao = 0;
    GLuint quad_vbo = 0;

    Blend() {

    }

    ~Blend() {
        destroy();
    }

    void init() {
        God::shaders.create_if_needed(std::string(SHADERS_DIR), "blend");
        target.init(God::context.render_target.width, God::context.render_target.height);
        initialize_quad();
    }

    void render() {

        target.clear(0.05f, 0.05f, 0.08f, 0.0f);

        for (auto group : God::layers.groups) {

            // No blend, rendering in target directly
            if (group.blend_op == 0) {
                target.bind();
                // Normal rendering of layers of current group
                for (int l = 0; l < God::layers.size(); ++l) {
                    Layer &layer = God::layers[l];
                    if (layer.group == group.id)
                        layer.render();
                }

            } else {

                RenderTarget cur_target;
                cur_target.init(target.width, target.height);

                // Bind framebuffer for offscreen rendering
                if (!cur_target.valid()) {
                    Log::error("RenderTarget is invalid");
                    break;
                }

                // clear & bind
                cur_target.clear(0.05f, 0.05f, 0.08f, 0.0f);

                // Normal rendering of layers of current group
                for (int l = 0; l < God::layers.size(); ++l) {
                    Layer &layer = God::layers[l];
                    if (layer.group == group.id)
                        layer.render();
                }

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                blend(target, cur_target, group.blend_op);

                // TODO necessary ?
                // Unbind framebuffer to render to screen
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, cur_target.width, cur_target.height); // Reset viewport

            }
        }

    }

    void display() {
        target.copy_to(God::context.render_target);
    }

    void blend(RenderTarget &target_a, RenderTarget &target_b, LayerManager::Group::BlendOp blend_op) {
        
        target_a.bind();
        // target_a.clear(0.05f, 0.05f, 0.08f, 1.0f);

        // Use blend shader
        GLuint blend_program = God::shaders["blend"];
        glUseProgram(blend_program);
        
        glUniform1i(glGetUniformLocation(blend_program, "mix"), static_cast<int>(blend_op));

        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, target_a.color);
        glUniform1i(glGetUniformLocation(blend_program, "tex_a"), 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, target_b.color);
        glUniform1i(glGetUniformLocation(blend_program, "tex_b"), 1);
        
        draw_quad();
    }

    // void render() {

    //     target.clear(0.05f, 0.05f, 0.08f, 1.0f);

    //     RenderTarget cur_target;
    //     cur_target.init(target.width, target.height);

    //     for (auto group : God::layers.groups) {


    //         // Bind framebuffer for offscreen rendering
    //         if (!cur_target.valid()) {
    //             Log::error("RenderTarget is invalid");
    //             break;
    //         }

    //         // TODO maybe only clear color to keep symbolic rendering
    //         cur_target.clear(0.05f, 0.05f, 0.08f, 1.0f);

    //         // Normal rendering of layers of current group
    //         for (int l = 0; l < God::layers.size(); ++l) {
    //             Layer &layer = God::layers[l];
    //             if (layer.group == group.id)
    //                 layer.render();
    //         }

    //         glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //         if (group.blend_op > 0)
    //             blend(cur_target, target, group.blend_op);

    //         // TODO necessary ?
    //         // Unbind framebuffer to render to screen
    //         glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //         glViewport(0, 0, cur_target.width, cur_target.height); // Reset viewport
    //     }

    // }

    // void display() {
    //     target.copy_to(God::context.render_target);
    // }

    // void blend(RenderTarget &target_a, RenderTarget &target_b, LayerManager::Group::BlendOp blend_op) {
        
    //     target_a.bind();
    //     // target_a.clear(0.05f, 0.05f, 0.08f, 1.0f);

    //     // Use blend shader
    //     GLuint blend_program = God::shaders["blend"];
    //     glUseProgram(blend_program);
        
    //     glUniform1i(glGetUniformLocation(blend_program, "mix"), static_cast<int>(blend_op));

    //     // Bind textures
    //     glActiveTexture(GL_TEXTURE0);
    //     glBindTexture(GL_TEXTURE_2D, target_a.color);
    //     glUniform1i(glGetUniformLocation(blend_program, "tex_a"), 0);
        
    //     glActiveTexture(GL_TEXTURE1);
    //     glBindTexture(GL_TEXTURE_2D, target_b.color);
    //     glUniform1i(glGetUniformLocation(blend_program, "tex_b"), 1);
        
    //     draw_quad();
    // }

private:

    RenderTarget target;

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

