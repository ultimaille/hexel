#include "render_target_composition.h"

void RenderTargetComposition::init(int w, int h) {
    God::shaders.create_if_needed(std::string(SHADERS_DIR), "blend");
    composite.init(w, h);
    initialize_quad();
    width = w;
    height = h;
}

void RenderTargetComposition::blend(GLuint a, GLuint b, Group::BlendOp blend_op) {
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

void RenderTargetComposition::begin() {
    for (int i = 0; i < God::layers.groups.size(); ++i) {
        auto &group = God::layers.groups[i];

        // Only create a new render target if needed: 
        // first group or when a merge option selected
        if (i == 0 || group.blend_op != Group::BlendOp::NONE) {
            targets[group.id].init(width, height);
        }
    }
}

void RenderTargetComposition::bind(int group_id) {
    // Only bind framebuffer if the target exists for group
    // This behavior implicitly don't bind if there is no blending,
    // enabling drawing in the same render target
    if (targets.contains(group_id)) {
        targets[group_id].clear(); // clear & bind framebuffer
        cur_target = group_id;
    }
}

void RenderTargetComposition::end() {
    if (targets.size() == 0)
        return;

    composite.resize(width, height);
    composite.clear();
    // Merge all color attachments into main render target, keeping its depth
    glBindFramebuffer(GL_FRAMEBUFFER, composite.framebuffer);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);  // Don't write to depth buffer
    glDepthFunc(GL_ALWAYS); // Pass all depth tests (use existing depth)

    // Render all framebuffer colors as layers        
    for (int i = 0; i < God::layers.groups.size(); ++i) {
        auto &group = God::layers.groups[i];
            if (targets.contains(group.id))
                blend(composite.color, targets[group.id].color, group.blend_op);
    }

    // Re-enable depth writes
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);

    // Clean up temporary targets
    targets.clear();
}

void RenderTargetComposition::destroy() {

    targets.clear();
    composite.destroy();

    if (quad_vbo != 0) {
        glDeleteBuffers(1, &quad_vbo);
        quad_vbo = 0;
    }

    if (quad_vao != 0) {
        glDeleteVertexArrays(1, &quad_vao);
        quad_vao = 0;
    }
}